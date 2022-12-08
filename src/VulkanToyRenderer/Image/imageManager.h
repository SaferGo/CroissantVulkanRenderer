#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Command/CommandPool.h>

namespace imageManager
{
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
         const VkComponentSwizzle& componentMapR,
         const VkComponentSwizzle& componentMapG,
         const VkComponentSwizzle& componentMapB,
         const VkComponentSwizzle& componentMapA,
         VkImageView& imageView
   );
   template<typename T>
   void copyDataToImage(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkDeviceSize size,
         const uint32_t offset,
         T* data,
         const uint32_t width,
         const uint32_t height,
         const VkFormat& format,
         const uint32_t mipLevels,
         const bool isCubemap,
         const VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool,
         const VkImage& image
   );

   void transitionImageLayout(
         const VkFormat& format,
         const uint32_t mipLevels,
         const VkImageLayout& oldLayout,
         const VkImageLayout& newLayout,
         const bool isCubemap,
         const std::shared_ptr<CommandPool>& commandPool,
         const VkQueue& graphicsQueue,
         const VkImage& image
   );
};
