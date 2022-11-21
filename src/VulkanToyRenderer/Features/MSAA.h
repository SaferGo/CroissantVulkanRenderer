#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Images/Image.h>

class MSAA
{

public:

   MSAA();
   MSAA(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkExtent2D& swapchainExtent,
         const VkFormat& swapchainFormat
   );
   ~MSAA();

   const VkSampleCountFlagBits& getSamplesCount() const;
   const VkImageView& getImageView() const;

   void destroy();

private:

   VkDevice              m_logicalDevice;

   Image                 m_image;
   VkSampleCountFlagBits m_samplesCount;

};

