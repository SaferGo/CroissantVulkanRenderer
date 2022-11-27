#include <VulkanToyRenderer/BufferManager/bufferUtils.h>

#include <stdexcept>

#include <vulkan/vulkan.h>

/*
 * Searchs for a suitable memory type with the indicated properties.
 */
uint32_t bufferUtils::findMemoryType(
      const VkPhysicalDevice& physicalDevice,
      const uint32_t typeFilter,
      const VkMemoryPropertyFlags& properties
) {
   VkPhysicalDeviceMemoryProperties memProperties;
   vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

   for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
   {
      bool isMemoryTypeSuitable = typeFilter & (1 << i);
      bool hasMemoryDesiredProperties = (
            memProperties.memoryTypes[i].propertyFlags & properties
      ) == properties;

      if (isMemoryTypeSuitable && hasMemoryDesiredProperties)
         return i;
   }

   throw std::runtime_error("Failed to find suitable memory type for buffer");
}


