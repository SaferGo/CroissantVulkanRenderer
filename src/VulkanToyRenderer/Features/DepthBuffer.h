
#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Image/Image.h>

class DepthBuffer
{

public:

   DepthBuffer();
   DepthBuffer(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkExtent2D& swapchainExtent,
      const VkSampleCountFlagBits& samplesCount
   );
   ~DepthBuffer();
   const VkImageView& getImageView() const;
   const VkFormat& getFormat() const;

   void destroy();

private:

   VkDevice m_logicalDevice;
   Image m_image;
   VkFormat m_format;

};
