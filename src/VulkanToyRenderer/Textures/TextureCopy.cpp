#include <VulkanToyRenderer/Textures/Texture.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Textures/mipmapUtils.h>
#include <VulkanToyRenderer/Textures/Bitmap.h>
#include <VulkanToyRenderer/Textures/cubemapUtils.h>
#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Images/imageManager.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Commands/commandUtils.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>

/*
 * Creates all the texture resources.
 */
Texture::Texture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
) : m_logicalDevice(logicalDevice),
    m_isCubemap(false),
    m_samplesCount(samplesCount)
{

   createTextureImage(
      (std::string(MODEL_DIR) + textureInfo.name).c_str(),
      textureInfo.format,
      physicalDevice,
      commandPool,
      graphicsQueue
   );
}

/*
 * Creates all the texture resources for a skybox(cubemap).
 */
Texture::Texture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const std::string& textureFolderName,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
) : m_logicalDevice(logicalDevice),
    m_isCubemap(true),
    m_samplesCount(samplesCount)
{
   createTextureImageCubemap(
         std::string(SKYBOX_DIR) + textureFolderName,
         textureInfo,
         physicalDevice,
         commandPool,
         graphicsQueue
   );
}


Texture::~Texture() {}

void Texture::transitionImageLayout(
      const VkFormat& format,
      const VkImageLayout& oldLayout,
      const VkImageLayout& newLayout,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
) {
   VkCommandBuffer commandBuffer;

   commandPool->allocCommandBuffer(commandBuffer, true);

   commandPool->beginCommandBuffer(
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
      imgMemoryBarrier.image = m_image.get();
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
            
   commandPool->endCommandBuffer(commandBuffer);

   commandPool->submitCommandBuffer(
         graphicsQueue,
         commandBuffer
   );
}

void Texture::createTextureImage(
      const char* pathToTexture,
      const VkFormat& format,
      const VkPhysicalDevice& physicalDevice,
      const std::shared_ptr<CommandPool>& commandPool,
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
         m_logicalDevice,
         imageSize,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         stagingBufferMemory,
         stagingBuffer
   );

   bufferManager::fillBuffer(
         m_logicalDevice,
         pixels,
         0,
         imageSize,
         stagingBufferMemory
   );
   
   stbi_image_free(pixels);

   m_image = Image(
         physicalDevice,
         m_logicalDevice,
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
         m_samplesCount,
         VK_IMAGE_ASPECT_COLOR_BIT,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_SAMPLER_ADDRESS_MODE_REPEAT,
         VK_FILTER_LINEAR
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
         stagingBuffer,
         commandPool,
         m_image.get()
   );

   vkDestroyBuffer(
         m_logicalDevice,
         stagingBuffer,
         nullptr
   );
   vkFreeMemory(
         m_logicalDevice,
         stagingBufferMemory,
         nullptr
   );
   
   mipmapUtils::generateMipmaps(
         physicalDevice,
         commandPool,
         graphicsQueue,
         m_image.get(),
         texWidth,
         texHeight,
         format,
         m_mipLevels
   );
}

void Texture::createTextureImageCubemap(
      const std::string& pathToTexture,
      const TextureToLoadInfo& textureInfo,
      const VkPhysicalDevice& physicalDevice,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
) {

   // Loads the texture

   int texWidth, texHeight, texChannels;
   VkDeviceSize imageSize;
   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;

   const float* img = stbi_loadf(
         (pathToTexture + "/" + textureInfo.name).c_str(),
         &texWidth,
         &texHeight,
         &texChannels,
         4
   );

   if (img == nullptr)
   {
      throw std::runtime_error(
            "Failed to load texture image: " +
            std::string(pathToTexture) + "/" +
            textureInfo.name
      );
   }

   // Creates a Vertical Cross texture from the square texture.
   
   Bitmap in(texWidth, texHeight, 4, eBitmapFormat_Float, img);
   Bitmap out = cubemapUtils::convertEquirectangularMapToVerticalCross(in);

   stbi_image_free((void*)img);

   stbi_write_hdr(
         (pathToTexture + "/verticalCross.hdr").c_str(),
         out.w_,
         out.h_,
         out.comp_,
         (const float*)out.data_.data()
   );

   // Divides the cross and creates the 6 faces.

   Bitmap cubemap = cubemapUtils::convertVerticalCrossToCubeMapFaces(out);

   uint8_t* data = cubemap.data_.data();
   imageSize = (
         cubemap.w_ *
         cubemap.h_ *
         // rgba
         cubemap.comp_ *
         Bitmap::getBytesPerComponent(cubemap.fmt_)
   );
   for (int i = 0; i < 6; i++)
   {
      stbi_write_hdr(
            (
             pathToTexture + "/" +
             config::TEXTURE_CUBEMAP_NAMING_CONV[i] +
             ".hdr"
            ).c_str(),
            cubemap.w_,
            cubemap.h_,
            cubemap.comp_,
            (const float*)data
      );
      data += imageSize;
   }

   // Creates the textures of the faces.
   
   stbi_uc* pixels[6];

   for (size_t i = 0; i < 6; i++)
   {
      pixels[i] = stbi_load(
            (
               std::string(pathToTexture) + "/" +
               config::TEXTURE_CUBEMAP_NAMING_CONV[i] +
               ".hdr"
            ).c_str(),
            &texWidth,
            &texHeight,
            &texChannels,
            4
      );
      std::cout << "TEX WIDTH ::: " << texWidth << std::endl;
      std::cout << "TEX HEIGHT ::: " << texHeight << std::endl;

      if (!pixels[i])
      {
         throw std::runtime_error(
               "Failed to load texture image: " +
               std::string(pathToTexture) + "/" +
               config::TEXTURE_CUBEMAP_NAMING_CONV[i] +
               ".hdr"
         );
      }
   }

   m_mipLevels = 1;

   imageSize = (
         texWidth *
         texHeight *
         // rgba
         4 *
         // 6 -> since we've 6 layers because of the cubemap.
         6
   );
   std::cout << "IMG WIDTH ::: " << cubemap.w_ << std::endl;
   std::cout << "IMG HEIGHT ::: " << cubemap.h_ << std::endl;
   std::cout << "IMG COMP ::: " << cubemap.comp_ << std::endl;

   bufferManager::createBuffer(
         physicalDevice,
         m_logicalDevice,
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
            m_logicalDevice,
            pixels[i],
            layerSize * i,
            layerSize,
            stagingBufferMemory
      );

      stbi_image_free(pixels[i]);
   }
   
   m_image = Image(
         physicalDevice,
         m_logicalDevice,
         cubemap.w_,
         cubemap.h_,
         textureInfo.format,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         true,
         m_mipLevels,
         m_samplesCount,
         VK_IMAGE_ASPECT_COLOR_BIT,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_SAMPLER_ADDRESS_MODE_REPEAT,
         VK_FILTER_LINEAR
   );
   
   // We will transfer the pixels to the image object with a cmd buffer.
   // (staging buffer to the image obj)
   transitionImageLayout(
         textureInfo.format,
         // Since the image was created with the VK_IMAGE_LAYOUT_UNDEFINED
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         commandPool,
         graphicsQueue
   );
   imageManager::copyBufferToImage(
         static_cast<uint32_t>(cubemap.w_),
         static_cast<uint32_t>(cubemap.h_),
         true,
         graphicsQueue,
         stagingBuffer,
         commandPool,
         m_image.get()
   );

   // Another transition to sample from the shader.
   transitionImageLayout(
         textureInfo.format,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         commandPool,
         graphicsQueue
   );

   vkDestroyBuffer(
         m_logicalDevice,
         stagingBuffer,
         nullptr
   );
   vkFreeMemory(
         m_logicalDevice,
         stagingBufferMemory,
         nullptr
   );
   
}

const VkImageView& Texture::getTextureImageView() const
{
   return m_image.getImageView();
}

const VkSampler& Texture::getTextureSampler() const
{
   return m_image.getSampler();
}

void Texture::destroy()
{
   m_image.destroy();
}

