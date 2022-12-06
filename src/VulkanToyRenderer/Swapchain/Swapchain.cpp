#include <VulkanToyRenderer/Swapchain/Swapchain.h>

#include <algorithm>
#include <iostream>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Queue/QueueFamilyIndices.h>
#include <VulkanToyRenderer/Image/imageManager.h>
#include <VulkanToyRenderer/Window/Window.h>
#include <VulkanToyRenderer/Features/MSAA.h>
#include <VulkanToyRenderer/Features/DepthBuffer.h>
#include <VulkanToyRenderer/Framebuffer/framebufferManager.h>

Swapchain::Swapchain() {}
Swapchain::~Swapchain() {}

Swapchain::Swapchain(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const std::shared_ptr<Window>& window,
      const SwapchainSupportedProperties& supportedProperties
) : m_logicalDevice(logicalDevice)
{

   VkSurfaceFormatKHR surfaceFormat;
   VkPresentModeKHR presentMode;
   VkExtent2D extent;

   chooseBestSettings(
         window,
         supportedProperties,
         surfaceFormat,
         presentMode,
         extent
   );

   m_imageFormat = surfaceFormat.format;
   m_extent = extent;
   
   // Chooses how many images we want to have in the swap chain.
   // (It's always recommended to request at least one more image that the
   // minimum because if we stick to this minimum, it means that we may
   // sometimes have to wait on the drive to complete internal operations
   // before we can acquire another imager to render to)
   m_minImageCount = (
         supportedProperties.capabilities.minImageCount
   );
   uint32_t imageCount = m_minImageCount + 1;

   bool isMaxResolution = existsMaxNumberOfSupportedImages(
         supportedProperties.capabilities
   );
   if (isMaxResolution == true &&
       imageCount > supportedProperties.capabilities.maxImageCount)
   {
      imageCount = supportedProperties.capabilities.maxImageCount;
   }

   VkSwapchainCreateInfoKHR createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   createInfo.surface = window->getSurface();
   createInfo.minImageCount = imageCount;
   createInfo.imageFormat = surfaceFormat.format;
   createInfo.imageColorSpace = surfaceFormat.colorSpace;
   createInfo.imageExtent = extent;
   // Specifies the amount of layers each image consists of.
   // (This is always 1 unless we are developing a stereoscopic 3D app)
   createInfo.imageArrayLayers = 1;
   // Specifies what kind of operations we'll use the image in the swap chain
   // for. Since in this app we are rendering directly to them, they are
   // used as color attachment.
   // (E.g: for post-processing -> VK_IMAGE_USAGE_TRANSFER_DST_BIT)
   createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

   // Configuration of how the images that are accessed from multiple queues
   // will be handled(we'll be drawing on the images in the swap chain from the
   // graphics queue and then submitting them on the presentation queue).
   //
   // - VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a
   //    time and ownership must be explicitly transferred before using it in
   //    another queue family. This option offers the best performance.
   // - VK_SHARING_MODE_CONCURRENT: Images can be used across mltiple queue
   //    families without explicit ownership transfers. Requires us to specify
   //    in advance between which queue families ownership wiil be shared
   //    using the queueFamilyIndexCount and pQueueFamilyIndices parameters.
   //
   // If the graphics queue family and presentation queue family are the same,
   // which will be the case on most hardware, then we should stick to
   // exclusive mode, because concurrent mode requires you to specify at least
   // two distinct queue families.
   QueueFamilyIndices indices;
   indices.getIndicesOfRequiredQueueFamilies(
         physicalDevice,
         window->getSurface()
   );
   uint32_t queueFamilyIndices[] = {
      indices.graphicsFamily.value(),
      indices.presentFamily.value()
   };

   if (indices.graphicsFamily != indices.presentFamily)
   {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
   } else
   {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      // Optinal
      createInfo.queueFamilyIndexCount = 0;
      // Optional
      createInfo.pQueueFamilyIndices = nullptr;
   }
   // We can specify that a certain transform should be applied to images in
   // the swap chain if it's supported(supportedTransofrms in capabilities),
   // like a 90 degree clockwsie rotation or horizontal flip.
   createInfo.preTransform = (
         supportedProperties.capabilities.currentTransform
   );

   // Specifies if the alpha channel should be used for mixing with other
   // windows in the window sysem. We'll almost always want to simply ignore
   // the alpha channel:
   createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

   createInfo.presentMode = presentMode;
   // We don't care abou the color pixels that are obscured, for example
   // because another window is in front of them. Unless you really need to be
   // able to read these pixels back an get predictable results, we'll get the
   // best performance by enabling clipping.
   createInfo.clipped = VK_TRUE;

   // Configures the old swapchain when the actual one becomes invaled or
   // unoptimized while the app is running(for example because the window was
   // resized).
   // For now we'll assume that we'll only ever create one swapchain.
   createInfo.oldSwapchain = VK_NULL_HANDLE;

   auto status = vkCreateSwapchainKHR(
         logicalDevice,
         &createInfo,
         nullptr,
         &m_swapchain
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create the Swapchain!");

  // Since in the creation of the swapchain we specified a minimum number of
  // images, in the implementation is allowed to create a swapchain with more.
  // That's why is necessary to query the final number of images.
  vkGetSwapchainImagesKHR(logicalDevice, m_swapchain, &imageCount, nullptr);
  m_images.resize(imageCount);
  vkGetSwapchainImagesKHR(
        logicalDevice,
        m_swapchain,
        &imageCount,
        m_images.data()
   );

  createAllImageViews();
}

void Swapchain::destroy()
{
   for (auto& framebuffer : m_framebuffers)
      vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);

   vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, nullptr);

   for (auto& imageView : m_imageViews)
      vkDestroyImageView(m_logicalDevice, imageView, nullptr);
}

void Swapchain::createAllImageViews()
{
  m_imageViews.resize(m_images.size());

  for (size_t i = 0; i < m_images.size(); i++)
   {
      imageManager::createImageView(
            m_logicalDevice,
            m_imageFormat,
            m_images[i],
            VK_IMAGE_ASPECT_COLOR_BIT,
            false,
            1,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            m_imageViews[i]
      );
   }
}

void Swapchain::createFramebuffers(
      const RenderPass& renderPass,
      const DepthBuffer& depthBuffer,
      const MSAA& msaa
) {
   m_framebuffers.resize(m_imageViews.size());

   // Iterates through each image view and creates the framebuffers.
   for (size_t i = 0; i < m_imageViews.size(); i++)
   {
      // Images in which we'll write in.
      std::vector<VkImageView> attachments = {
         msaa.getImageView(),
         depthBuffer.getImageView(),
         m_imageViews[i]
      };

      framebufferManager::createFramebuffer(
            m_logicalDevice,
            renderPass.get(),
            attachments,
            m_extent.width,
            m_extent.height,
            1,
            m_framebuffers[i]
      );
   }
}

const uint32_t Swapchain::getNextImageIndex(const VkSemaphore& semaphore) const
{
   uint32_t imageIndex;

   vkAcquireNextImageKHR(
         m_logicalDevice,
         m_swapchain,
         UINT64_MAX,
         // Specifies synchr. objects that have to be signaled when the
         // presentation engine is finished using the image.
         semaphore,
         VK_NULL_HANDLE,
         &imageIndex
   );

   return imageIndex;
}

const VkImageView& Swapchain::getImageView(const uint32_t index) const {
   return m_imageViews[index];
}

void Swapchain::chooseBestSettings(
      const std::shared_ptr<Window>& window,
      const SwapchainSupportedProperties& supportedProperties,
      VkSurfaceFormatKHR& surfaceFormat,
      VkPresentModeKHR& presentMode,
      VkExtent2D& extent
) {

   surfaceFormat = chooseBestSurfaceFormat(
         supportedProperties.surfaceFormats
   );
   presentMode = chooseBestPresentMode(
         supportedProperties.presentModes
   );

   extent = chooseBestExtent(
         supportedProperties.capabilities,
         window
   );
}

/*
 * Chooses the surface format that we want(if it's available), if
 * it's not available, then any other option is chosen.
 */
VkSurfaceFormatKHR Swapchain::chooseBestSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats
) {
   // sRGB -> gamma correction.
   for (const auto& availableFormat : availableFormats)
   {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
          availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      {
         return availableFormat;
      }
   }

   // If the availables formats don't fit in what are we looking for, we can
   // just return any available option. In this case, the first one.
   return availableFormats[0];
}

/*
 * Chooses the surface present mode that we want(if it's available), if
 * it's not available, then the option "VK_PRESENT_MODE_FIFO_KHR" is chosen.
 *
 * - VK_PRESENT_MODE_MAILBOX_KHR -> Best option(in this case). Instead of 
 *       blocking the application when the queue is full, the images that are
 *       already queued are simply replaced with the newer ones. This mode can
 *       be used to render frames as fast as possible while still avoiding
 *       tearing, resulting in fewer latency issues than standard vertical 
 *       sync. This is commonly known as "triple buffering", although the
 *       existence of three buffers alone does not necessarily mean that the
 *       framerate is unlocked.
 *
 * - VK_PRESENT_MODE_FIFO_KHR    -> This present mode is guaranteed to be 
 *       always available.
 */
VkPresentModeKHR Swapchain::chooseBestPresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes
) {
   for (const auto& availablePresentMode : availablePresentModes)
   {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
         return availablePresentMode;
   }

   return VK_PRESENT_MODE_FIFO_KHR;
}

/*
 * Chooses the best resolution that more matches the one of the window surface.
 */
VkExtent2D Swapchain::chooseBestExtent(
      const VkSurfaceCapabilitiesKHR& capabilities,
      const std::shared_ptr<Window>& window
) {
   if (window->isAllowedToModifyTheResolution(capabilities) == false)
      return capabilities.currentExtent;

   int width, height;
   window->getResolutionInPixels(width,height);

   VkExtent2D actualExtent =
   {
      static_cast<uint32_t>(width),
      static_cast<uint32_t>(height)
   };

   actualExtent.width = std::clamp(
         actualExtent.width,
         capabilities.minImageExtent.width,
         capabilities.maxImageExtent.width
   );
   actualExtent.height = std::clamp(
         actualExtent.height,
         capabilities.minImageExtent.height,
         capabilities.maxImageExtent.height
   );


   return actualExtent;
}

const VkExtent2D& Swapchain::getExtent() const
{
   return m_extent;
}

const VkFramebuffer& Swapchain::getFramebuffer(const uint32_t imageIndex) const
{
   return m_framebuffers[imageIndex];
}

const VkFormat& Swapchain::getImageFormat() const
{
   return m_imageFormat;
}

const VkSwapchainKHR& Swapchain::get() const
{
   return m_swapchain;
}

void Swapchain::presentImage(
      const uint32_t imageIndex,
      const std::vector<VkSemaphore> signalSemaphores,
      const VkQueue& presentQueue
) {

   VkPresentInfoKHR presentInfo{};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   // Specifies which semaphores to wait on before the presentation can happen.
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = signalSemaphores.data();

   // Specifies the swapchains to present images to and the index of the
   // image for each swapchain.
   //VkSwapchainKHR swapchains[] = {m_swapchain};
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = &m_swapchain;
   presentInfo.pImageIndices = &imageIndex;
   // Allows us to specify an array of VkResult values to check for every
   // individual swapchain if presentation was successful.
   // Optional
   presentInfo.pResults = nullptr;

   vkQueuePresentKHR(presentQueue, &presentInfo);
}

/*
 * If the maxImageCount is equal to 0, it means that there is no maximum.
 */
const bool Swapchain::existsMaxNumberOfSupportedImages(
      const VkSurfaceCapabilitiesKHR& capabilities
) const {
   return (capabilities.maxImageCount != 0);
}

const uint32_t Swapchain::getImageCount() const
{
   return static_cast<uint32_t>(m_images.size());
}

const uint32_t Swapchain::getMinImageCount() const
{
   return m_minImageCount;
}
