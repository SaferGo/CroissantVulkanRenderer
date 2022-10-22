#include <VulkanToyRenderer/GraphicsPipeline/renderTarget.h>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Images/imageManager.h>
#include <VulkanToyRenderer/GraphicsPipeline/renderTargetUtils.h>


///////////////////////////////////MSAA////////////////////////////////////////

renderTarget::MSAA::MSAA() {}

renderTarget::MSAA::MSAA(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkExtent2D& swapchainExtent,
      const VkFormat& swapchainFormat
) {
   m_samplesCount = renderTargetUtils::msaa::getMaxUsableSampleCount(
         physicalDevice
   );

   imageManager::createImage(
         physicalDevice,
         logicalDevice,
         swapchainExtent.width,
         swapchainExtent.height,
         swapchainFormat,
         VK_IMAGE_TILING_OPTIMAL,
         (
            VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
         ),
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         false,
         1,
         m_samplesCount,
         m_image,
         m_imageMemory
   );

 
   imageManager::createImageView(
         logicalDevice,
         swapchainFormat,
         m_image,
         VK_IMAGE_ASPECT_COLOR_BIT,
         false,
         1,
         m_imageView
   ); 
}

renderTarget::MSAA::~MSAA() {}


const VkSampleCountFlagBits& renderTarget::MSAA::getSamplesCount() const
{
   return m_samplesCount;
}

void renderTarget::MSAA::destroy(
      const VkDevice& logicalDevice
) {
   vkDestroyImageView(logicalDevice, m_imageView, nullptr);
   vkDestroyImage(logicalDevice, m_image, nullptr);
   vkFreeMemory(logicalDevice, m_imageMemory, nullptr);
}

const VkImageView& renderTarget::MSAA::getImageView() const
{
   return m_imageView;
}

/////////////////////////////////Depth buffer//////////////////////////////////

renderTarget::DepthBuffer::DepthBuffer() {}

renderTarget::DepthBuffer::DepthBuffer(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkExtent2D& swapchainExtent,
      const VkSampleCountFlagBits& samplesCount
) {
   m_format = renderTargetUtils::depthBuffer::findSupportedFormat(
         physicalDevice,
         {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
         },
         VK_IMAGE_TILING_OPTIMAL,
         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
   );

   imageManager::createImage(
         physicalDevice,
         logicalDevice,
         swapchainExtent.width,
         swapchainExtent.height,
         m_format,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         false,
         1,
         samplesCount,
         m_image,
         m_imageMemory
   );

   imageManager::createImageView(
         logicalDevice,
         m_format,
         m_image,
         VK_IMAGE_ASPECT_DEPTH_BIT,
         false,
         1,
         m_imageView
   );
}

renderTarget::DepthBuffer::~DepthBuffer() {}

const VkImageView& renderTarget::DepthBuffer::getImageView() const
{
   return m_imageView;
}

const VkFormat& renderTarget::DepthBuffer::getFormat() const
{
   return m_format;
}

void renderTarget::DepthBuffer::destroy(
      const VkDevice& logicalDevice
) {
   vkDestroyImageView(logicalDevice, m_imageView, nullptr);
   vkDestroyImage(logicalDevice, m_image, nullptr);
   vkFreeMemory(logicalDevice, m_imageMemory, nullptr);
}
