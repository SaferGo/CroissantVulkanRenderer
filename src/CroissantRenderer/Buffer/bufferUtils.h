#pragma once

#include <vulkan/vulkan.h>

namespace bufferUtils
{
   uint32_t findMemoryType(
      const VkPhysicalDevice& physicalDevice,
      const uint32_t typeFilter,
      const VkMemoryPropertyFlags& properties
   );
};
