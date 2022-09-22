#include <VulkanToyRenderer/Textures/Texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Images/imageManager.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Commands/commandUtils.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/Sampler.h>

Texture::Texture() {}
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
      imgMemoryBarrier.subresourceRange.levelCount = 1;
      // In this case, our image is not an array(it has 2D coords -> texel).
      imgMemoryBarrier.subresourceRange.baseArrayLayer = 0;
      imgMemoryBarrier.subresourceRange.layerCount = 1;
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
         logicalDevice
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
         m_textureImageView
   );

}

void Texture::createTextureImage(
      const char* pathToTexture,
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
) {
   int texWidth, texHeight, texChannels;

   stbi_uc* pixels = stbi_load(
         pathToTexture,
         &texWidth,
         &texHeight,
         &texChannels,
         STBI_rgb_alpha
   );
   // *4 -> Because we are forcing the image to have RGBA.
   VkDeviceSize imageSize = texWidth * texHeight * 4;

   if (!pixels)
      throw std::runtime_error(
            "Failed to load texture image: " + std::string(pathToTexture)
      );

   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   
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

   void *data;
   vkMapMemory(
         logicalDevice,
         stagingBufferMemory,
         0,
         imageSize,
         0,
         &data
   );
      mempcpy(data, pixels, static_cast<size_t>(imageSize));
   vkUnmapMemory(
         logicalDevice,
         stagingBufferMemory
   );
   
   stbi_image_free(pixels);

   // Creates an empty Image object of determined properties.
   imageManager::createImage(
         physicalDevice,
         logicalDevice,
         texWidth,
         texHeight,
         VK_FORMAT_R8G8B8A8_SRGB,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         m_textureImage,
         m_textureImageMemory
   );
   
   // We will transfer the pixels to the image object with a cmd buffer.
   // (staging buffer to the image obj)
   transitionImageLayout(
         VK_FORMAT_R8G8B8A8_SRGB,
         // Since the image was created with the VK_IMAGE_LAYOUT_UNDEFINED
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         commandPool,
         graphicsQueue
   );
   imageManager::copyBufferToImage(
         static_cast<uint32_t>(texWidth),
         static_cast<uint32_t>(texHeight),
         graphicsQueue,
         commandPool,
         stagingBuffer,
         m_textureImage
   );
   // Another transition to sample from the shader.
   transitionImageLayout(
         VK_FORMAT_R8G8B8A8_SRGB,
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
