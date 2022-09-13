#pragma once

#include <vulkan/vulkan.h>

class DepthBuffer
{

public:

   DepthBuffer();
   ~DepthBuffer();

   void createDepthBuffer(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkExtent2D swapchainExtent
   );
   const VkImageView getDepthImageView() const;

private:

   VkImage m_depthImage;
   VkDeviceMemory m_depthImageMemory;
   VkImageView m_depthImageView;

};
