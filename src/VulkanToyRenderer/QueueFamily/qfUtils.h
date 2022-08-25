#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace qfUtils
{
   bool isGraphicsQueueSupported(const VkQueueFamilyProperties& qfSupported);

   bool isPresentQueueSupported(
         const int qfSupportedIndex,
         const VkSurfaceKHR& surface,
         const VkPhysicalDevice& device
   );

   void getSupportedQueueFamilies(
      const VkPhysicalDevice& device,
      std::vector<VkQueueFamilyProperties>& qfSupported
   );
};
