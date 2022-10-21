#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Commands/CommandPool.h>

namespace mipmapUtils
{
   void generateMipmaps(
         const VkPhysicalDevice& physicalDevice,
         CommandPool& commandPool,
         VkQueue& graphicsQueue,
         VkImage& image,
         const int32_t width,
         const int32_t height,
         const VkFormat& format,
         const int32_t mipLevels
   );

   bool isLinearBlittingSupported(
         const VkPhysicalDevice& physicalDevice,
         const VkFormat& format
   );

   const int32_t getAmountOfSupportedMipLevels(
         const int32_t width,
         const int32_t height
   );

};
