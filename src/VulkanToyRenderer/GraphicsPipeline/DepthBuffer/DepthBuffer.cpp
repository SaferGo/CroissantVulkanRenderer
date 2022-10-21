#include <VulkanToyRenderer/GraphicsPipeline/DepthBuffer/DepthBuffer.h>

#include <VulkanToyRenderer/Images/imageManager.h>
#include <VulkanToyRenderer/GraphicsPipeline/DepthBuffer/depthUtils.h>

DepthBuffer::DepthBuffer() {}
DepthBuffer::~DepthBuffer() {}

void DepthBuffer::createDepthBuffer(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkExtent2D swapchainExtent
) {
   VkFormat depthFormat = depthUtils::findSupportedFormat(
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
         depthFormat,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         false,
         1,
         m_depthImage,
         m_depthImageMemory
   );

   imageManager::createImageView(
         logicalDevice,
         depthFormat,
         m_depthImage,
         VK_IMAGE_ASPECT_DEPTH_BIT,
         false,
         1,
         m_depthImageView
   );
}


const VkImageView DepthBuffer::getDepthImageView() const
{
   return m_depthImageView;
}


void DepthBuffer::destroyDepthBuffer(const VkDevice& logicalDevice)
{
   vkDestroyImageView(logicalDevice, m_depthImageView, nullptr);
   vkDestroyImage(logicalDevice, m_depthImage, nullptr);
   vkFreeMemory(logicalDevice, m_depthImageMemory, nullptr);
}
