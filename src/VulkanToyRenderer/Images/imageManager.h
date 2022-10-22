#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Commands/CommandPool.h>

namespace imageManager{
   void createImage(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t width,
      const uint32_t height,
      const VkFormat& format,
      const VkImageTiling& tiling,
      const VkImageUsageFlags& usage,
      const VkMemoryPropertyFlags& memoryProperties,
      const bool isCubemap,
      const uint32_t mipLevels,
      const VkSampleCountFlagBits& numSamples,
      VkImage& image,
      VkDeviceMemory& memory
   );
   void createImageView(
         const VkDevice& logicalDevice,
         const VkFormat& format,
         const VkImage& image,
         const VkImageAspectFlags& aspectFlags,
         const bool isCubemap,
         const uint32_t mipLevels,
         VkImageView& imageView
   );
   void copyBufferToImage(
         const uint32_t width,
         const uint32_t height,
         const bool isCubemap,
         VkQueue& graphicsQueue,
         CommandPool& commandPool,
         VkBuffer& buffer,
         VkImage& image
   );
};
