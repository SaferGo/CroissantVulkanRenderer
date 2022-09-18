#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/Sampler.h>

class Texture
{

public:

   Texture();
   ~Texture();
   
   void createTextureImage(
         const char* pathToTexture,
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
   
   const VkImageView& getTextureImageView() const;
   const VkSampler& getTextureSampler() const;

   void destroyTexture(const VkDevice& logicalDevice);

private:

   void transitionImageLayout(
         const VkFormat& format,
         const VkImageLayout& oldLayout,
         const VkImageLayout& newLayout,
         CommandPool& commandPool,
         VkQueue& graphicsQueue
   );


   VkImage        m_textureImage;
   VkDeviceMemory m_textureImageMemory;
   VkImageView    m_textureImageView;

   // Descriptor
   Sampler        m_textureSampler;

};
