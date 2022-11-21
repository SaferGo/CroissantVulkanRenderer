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
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
   );
   Texture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const bool isIrradianceMap,
      const TextureToLoadInfo& textureInfo,
      const std::string& textureFolderName,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
   );
   ~Texture();
   
      
   const VkImageView& getImageView() const;
   const VkSampler& getSampler() const;

   void destroy();

private:

   void createTextureImage(
         const char* pathToTexture,
         const VkFormat& format,
         const VkPhysicalDevice& physicalDevice,
         const std::shared_ptr<CommandPool>& commandPool,
         VkQueue& graphicsQueue
   );
   void createTextureImageView(const VkFormat& format);
   void createTextureSampler(const VkPhysicalDevice& physicalDevice);
   void transitionImageLayout(
         const VkFormat& format,
         const VkImageLayout& oldLayout,
         const VkImageLayout& newLayout,
         const std::shared_ptr<CommandPool>& commandPool,
         VkQueue& graphicsQueue
   );

   void createTextureImageCubemap(
         const std::string& pathToTexture,
         const TextureToLoadInfo& textureInfo,
         const VkPhysicalDevice& physicalDevice,
         const std::shared_ptr<CommandPool>& commandPool,
         VkQueue& graphicsQueue
   );

   VkDevice              m_logicalDevice;
   Image                 m_image;
   bool                  m_isCubemap;
   bool                  m_isIrradianceMap;
   uint32_t              m_mipLevels;
   VkSampleCountFlagBits m_samplesCount;
};
