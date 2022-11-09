#include <VulkanToyRenderer/Images/Image.h>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Images/imageManager.h>

Image::Image() {}

Image::Image(
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
) {
   init(
         physicalDevice,
         logicalDevice,
         width,
         height,
         format,
         tiling,
         usage,
         memoryProperties,
         isCubemap,
         mipLevels,
         numSamples,
         aspectFlags,
         componentMapR,
         componentMapG,
         componentMapB,
         componentMapA
   );
}

Image::Image(
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
) {
   init(
         physicalDevice,
         logicalDevice,
         width,
         height,
         format,
         tiling,
         usage,
         memoryProperties,
         isCubemap,
         mipLevels,
         numSamples,
         aspectFlags,
         componentMapR,
         componentMapG,
         componentMapB,
         componentMapA
   );

   m_sampler = Sampler(
         physicalDevice,
         logicalDevice,
         mipLevels,
         addressMode,
         filter
   );
}

void Image::init(
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
) {

   imageManager::createImage(
         physicalDevice,
         logicalDevice,
         width,
         height,
         format,
         tiling,
         usage,
         memoryProperties,
         isCubemap,
         mipLevels,
         numSamples,
         m_image,
         m_imageMemory
   );

   imageManager::createImageView(
         logicalDevice,
         format,
         m_image,
         aspectFlags,
         isCubemap,
         mipLevels,
         componentMapR,
         componentMapG,
         componentMapB,
         componentMapA,
         m_imageView
   );

}


Image::~Image() {}

const VkImage& Image::get() const
{
   return m_image;
}

const VkImageView& Image::getImageView() const
{
   return m_imageView;
}

const VkSampler& Image::getSampler() const
{
      return m_sampler->get();
}

void Image::destroy(const VkDevice& logicalDevice)
{
   if (m_sampler.has_value())
      m_sampler->destroySampler(logicalDevice);

   vkDestroyImageView(logicalDevice, m_imageView, nullptr);
   vkDestroyImage(logicalDevice, m_image, nullptr);
   vkFreeMemory(logicalDevice, m_imageMemory, nullptr);
}
