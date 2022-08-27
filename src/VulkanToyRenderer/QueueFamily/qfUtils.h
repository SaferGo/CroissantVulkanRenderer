#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace qfUtils
{
   bool isGraphicsQueueSupported(const VkQueueFamilyProperties& qfSupported);

   bool isPresentQueueSupported(
         const int qfSupportedIndex,
         const VkSurfaceKHR& surface,
         const VkPhysicalDevice& physicalDevice
   );

   void getSupportedQueueFamilies(
      const VkPhysicalDevice& physicalDevice,
      std::vector<VkQueueFamilyProperties>& qfSupported
   );
};
