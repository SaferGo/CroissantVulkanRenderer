#pragma once

#include <optional>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>

class Image
{

public:

   Image();
   // Creates image without Sampler
   Image(
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
      // Parameters for Image View.
      const VkImageAspectFlags& aspectFlags,
      const VkComponentSwizzle& componentMapR,
      const VkComponentSwizzle& componentMapG,
      const VkComponentSwizzle& componentMapB,
      const VkComponentSwizzle& componentMapA
   );
   // Creates image with sampler.
   Image(
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
      // Parameters for Image View.
      const VkImageAspectFlags& aspectFlags,
      const VkComponentSwizzle& componentMapR,
      const VkComponentSwizzle& componentMapG,
      const VkComponentSwizzle& componentMapB,
      const VkComponentSwizzle& componentMapA,
      // Parameters to create the Sampler
      const VkSamplerAddressMode& addressMode,
      const VkFilter& filter
   );
   void init(
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
      // Parameters for Image View.
      const VkImageAspectFlags& aspectFlags,
      const VkComponentSwizzle& componentMapR,
      const VkComponentSwizzle& componentMapG,
      const VkComponentSwizzle& componentMapB,
      const VkComponentSwizzle& componentMapA
   );
   ~Image();

   const VkImage& get() const;
   const VkImageView& getImageView() const;
   const VkSampler& getSampler() const;
   void destroy(const VkDevice& logicalDevice);

private:

   VkImage                 m_image;
   VkImageView             m_imageView;
   VkDeviceMemory          m_imageMemory;
   std::optional<Sampler>  m_sampler;

};
