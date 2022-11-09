#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Images/Image.h>

struct TextureToLoadInfo
{
   std::string name;
   VkFormat format;
};

class Texture
{

public:

   Texture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const bool isCubemap,
      const VkSampleCountFlagBits& samplesCount,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
   );
   ~Texture();
   
      
   const VkImageView& getTextureImageView() const;
   const VkSampler& getTextureSampler() const;

   void destroyTexture(const VkDevice& logicalDevice);

private:

   void createTextureImage(
         const char* pathToTexture,
         const VkFormat& format,
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         CommandPool& commandPool,
         VkQueue& graphicsQueue
   );
   void createTextureImageView(
         const VkDevice& logicalDevice,
         const VkFormat& format
   );
   void createTextureSampler(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice
   );
   void transitionImageLayout(
         const VkFormat& format,
         const VkImageLayout& oldLayout,
         const VkImageLayout& newLayout,
         CommandPool& commandPool,
         VkQueue& graphicsQueue
   );

   void createTextureImageCubemap(
         const char* pathToTexture,
         const VkFormat& format,
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         CommandPool& commandPool,
         VkQueue& graphicsQueue
   );

   Image                 m_image;
   bool                  m_isCubemap;
   uint32_t              m_mipLevels;
   VkSampleCountFlagBits m_samplesCount;
};
