#include <VulkanToyRenderer/Textures/Texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Textures/mipmapUtils.h>
#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Images/imageManager.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Commands/commandUtils.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>

Texture::Texture() {}

/*
 * Creates all the texture resources.
 */
Texture::Texture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const bool isCubemap,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
) : m_isCubemap(isCubemap)
{

   if (m_isCubemap)
   {
      createTextureImageCubemap(
            (std::string(SKYBOX_DIR) + textureInfo.name).c_str(),
            textureInfo.format,
            physicalDevice,
            logicalDevice,
            commandPool,
            graphicsQueue
      );
      
   } else {

      createTextureImage(
         (std::string(MODEL_DIR) + textureInfo.name).c_str(),
         textureInfo.format,
         physicalDevice,
         logicalDevice,
         commandPool,
         graphicsQueue
      );

   }

   createTextureImageView(
         logicalDevice,
         textureInfo.format
   );
   createTextureSampler(
         physicalDevice,
         logicalDevice
   );
}

Texture::~Texture() {}

void Texture::transitionImageLayout(
      const VkFormat& format,
      const VkImageLayout& oldLayout,
      const VkImageLayout& newLayout,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
) {
   VkCommandBuffer commandBuffer;

   commandPool.allocCommandBuffer(commandBuffer, true);

   commandPool.beginCommandBuffer(
         VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
         commandBuffer
   );

      VkImageMemoryBarrier imgMemoryBarrier{};
      imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      // We could use VK_IMAGE_LAYOUT_UNDEFINED if we don't care about the
      // existing contents of the image.
      imgMemoryBarrier.oldLayout = oldLayout;
      imgMemoryBarrier.newLayout = newLayout;
      // Since we are not using the barrier to transfer queue family ownership,
      // we'll ignore these two.
      imgMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      imgMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      imgMemoryBarrier.image = m_textureImage;
      imgMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imgMemoryBarrier.subresourceRange.baseMipLevel = 0;
      // In this case, our image is not an array(it has 2D coords -> texel).
      imgMemoryBarrier.subresourceRange.baseArrayLayer = 0;

      if (m_isCubemap)
      {
         imgMemoryBarrier.subresourceRange.layerCount = 6;
         imgMemoryBarrier.subresourceRange.levelCount = 1;
      } else
      {
         imgMemoryBarrier.subresourceRange.levelCount = m_mipLevels;
         imgMemoryBarrier.subresourceRange.layerCount = 1;
      }

      //Barriers are primarily used for synchronization purposes, so you must
      //specify which types of operations that involve the resource must happen
      //before the barrier, and which operations that involve the resource must
      //wait on the barrier. We need to do that despite already using
      //vkQueueWaitIdle to manually synchronize. The right values depend on the
      //old and new layout.
      imgMemoryBarrier.srcAccessMask = 0;
      imgMemoryBarrier.dstAccessMask = 0;

      // Defines the (pseudo)pipelines stages.
      // (this prevents that the transfer doesn't collide with the writing and
      // reading from other resources)
      VkPipelineStageFlags sourceStage;
      VkPipelineStageFlags destinationStage;

      if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
          newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      ) {

         imgMemoryBarrier.srcAccessMask = 0;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

         sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
         destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

      } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      ) {

         imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

         sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
         destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      } else
         throw std::invalid_argument("Unsupported layout transition!");


      commandUtils::SYNCHRONIZATION::recordPipelineBarrier(
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &imgMemoryBarrier,
            commandBuffer
      );
            
   commandPool.endCommandBuffer(commandBuffer);

   commandPool.submitCommandBuffer(
         graphicsQueue,
         commandBuffer
   );
}

void Texture::createTextureSampler(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice
) {
   m_textureSampler.createSampler(
         physicalDevice,
         logicalDevice,
         m_mipLevels
   );
}

void Texture::createTextureImageView(
      const VkDevice& logicalDevice,
      const VkFormat& format
) {
   imageManager::createImageView(
         logicalDevice,
         format,
         m_textureImage,
         VK_IMAGE_ASPECT_COLOR_BIT,
         m_isCubemap,
         m_mipLevels,
         m_textureImageView
   );

}

void Texture::createTextureImage(
      const char* pathToTexture,
      const VkFormat& format,
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
) {

   int texWidth, texHeight, texChannels;
   VkDeviceSize imageSize;
   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;

   stbi_uc* pixels = stbi_load(
         pathToTexture,
         &texWidth,
         &texHeight,
         &texChannels,
         STBI_rgb_alpha
   );
   if (!pixels)
   {
      throw std::runtime_error(
            "Failed to load texture image: " + std::string(pathToTexture)
      );
   }

   m_mipLevels = mipmapUtils::getAmountOfSupportedMipLevels(
         texWidth,
         texHeight
   );

   // *4 -> Because we are forcing the image to have RGBA.
   imageSize = texWidth * texHeight * 4;

   bufferManager::createBuffer(
         physicalDevice,
         logicalDevice,
         imageSize,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         stagingBufferMemory,
         stagingBuffer
   );

   bufferManager::fillBuffer(
         logicalDevice,
         pixels,
         0,
         imageSize,
         stagingBufferMemory
   );
   
   stbi_image_free(pixels);

   // Creates an empty Image object of determined properties.
   imageManager::createImage(
         physicalDevice,
         logicalDevice,
         texWidth,
         texHeight,
         format,
         VK_IMAGE_TILING_OPTIMAL,
         (
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT
         ),
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         false,
         m_mipLevels,
         m_textureImage,
         m_textureImageMemory
   );
   
   // We will transfer the pixels to the image object with a cmd buffer.
   // (staging buffer to the image obj)
   transitionImageLayout(
         format,
         // Since the image was created with the VK_IMAGE_LAYOUT_UNDEFINED
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         commandPool,
         graphicsQueue
   );

   imageManager::copyBufferToImage(
         static_cast<uint32_t>(texWidth),
         static_cast<uint32_t>(texHeight),
         false,
         graphicsQueue,
         commandPool,
         stagingBuffer,
         m_textureImage
   );

   vkDestroyBuffer(
         logicalDevice,
         stagingBuffer,
         nullptr
   );
   vkFreeMemory(
         logicalDevice,
         stagingBufferMemory,
         nullptr
   );
   
   mipmapUtils::generateMipmaps(
         physicalDevice,
         commandPool,
         graphicsQueue,
         m_textureImage,
         texWidth,
         texHeight,
         format,
         m_mipLevels
   );
}

void Texture::createTextureImageCubemap(
      const char* pathToTexture,
      const VkFormat& format,
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
) {

   int texWidth, texHeight, texChannels;
   VkDeviceSize imageSize;
   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;

   stbi_uc* pixels[6];

   for (size_t i = 0; i < 6; i++)
   {
      pixels[i] = stbi_load(
            (
               std::string(pathToTexture) + "/" +
               config::TEXTURE_CUBEMAP_NAMING_CONV[i] +
               "." +
               config::TEXTURE_CUBEMAP_FORMAT
            ).c_str(),
            &texWidth,
            &texHeight,
            &texChannels,
            STBI_rgb_alpha
      );

      if (!pixels[i])
      {
         throw std::runtime_error(
               "Failed to load texture image: " +
               std::string(pathToTexture) + "/" +
               config::TEXTURE_CUBEMAP_NAMING_CONV[i] +
               "." +
               config::TEXTURE_CUBEMAP_FORMAT
         );
      }
   }

   m_mipLevels = 1;

   // 4 -> rgbA
   // 6 -> since we've 6 layers because of the cubemap.
   imageSize = texWidth * texHeight * 4 * 6;

   bufferManager::createBuffer(
         physicalDevice,
         logicalDevice,
         imageSize,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         stagingBufferMemory,
         stagingBuffer
   );

   VkDeviceSize layerSize = imageSize / 6;
   for (size_t i = 0; i < 6; i++)
   {
      bufferManager::fillBuffer(
            logicalDevice,
            pixels[i],
            layerSize * i,
            layerSize,
            stagingBufferMemory
      );
      
      stbi_image_free(pixels[i]);
   }
   
   
   // Creates an empty Image object of determined properties.
   imageManager::createImage(
         physicalDevice,
         logicalDevice,
         texWidth,
         texHeight,
         format,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         true,
         m_mipLevels,
         m_textureImage,
         m_textureImageMemory
   );
   
   // We will transfer the pixels to the image object with a cmd buffer.
   // (staging buffer to the image obj)
   transitionImageLayout(
         format,
         // Since the image was created with the VK_IMAGE_LAYOUT_UNDEFINED
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         commandPool,
         graphicsQueue
   );
   imageManager::copyBufferToImage(
         static_cast<uint32_t>(texWidth),
         static_cast<uint32_t>(texHeight),
         true,
         graphicsQueue,
         commandPool,
         stagingBuffer,
         m_textureImage
   );

   // Another transition to sample from the shader.
   transitionImageLayout(
         format,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         commandPool,
         graphicsQueue
   );

   vkDestroyBuffer(
         logicalDevice,
         stagingBuffer,
         nullptr
   );
   vkFreeMemory(
         logicalDevice,
         stagingBufferMemory,
         nullptr
   );
   
}

const VkImageView& Texture::getTextureImageView() const
{
   return m_textureImageView;
}


const VkSampler& Texture::getTextureSampler() const
{
   return m_textureSampler.getSampler();
}

void Texture::destroyTexture(const VkDevice& logicalDevice)
{
   m_textureSampler.destroySampler(logicalDevice);
   vkDestroyImageView(logicalDevice, m_textureImageView, nullptr);
   vkDestroyImage(logicalDevice, m_textureImage, nullptr);
   vkFreeMemory(logicalDevice, m_textureImageMemory, nullptr);
}

