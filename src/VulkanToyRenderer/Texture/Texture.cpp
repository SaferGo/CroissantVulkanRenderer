#include <VulkanToyRenderer/Texture/Texture.h>

#include <iostream>

#include <vulkan/vulkan.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <VulkanToyRenderer/Texture/mipmapUtils.h>
#include <VulkanToyRenderer/Texture/Bitmap.h>
#include <VulkanToyRenderer/Command/commandManager.h>
#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Descriptor/Types/Sampler/Sampler.h>

Texture::Texture(
      const VkDevice& logicalDevice,
      const TextureType& type,
      const VkSampleCountFlagBits& samplesCount,
      const int& desiredChannels,
      const UsageType& usage
) : m_logicalDevice(logicalDevice),
    m_type(type),
    m_usage(usage),
    m_desiredChannels(desiredChannels),
    m_samplesCount(samplesCount)
{}

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

      if (m_type == TextureType::CUBEMAP)
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


      commandManager::synchronization::recordPipelineBarrier(
            sourceStage,
            destinationStage,
            0,
            commandBuffer,
            {},
            {},
            {imgMemoryBarrier}
      );
            
   commandPool->endCommandBuffer(commandBuffer);

   commandPool->submitCommandBuffer(
         graphicsQueue,
         {commandBuffer},
         true
   );
}

const VkImageView& Texture::getImageView() const
{
   return m_image.getImageView();
}

const VkSampler& Texture::getSampler() const
{
   return m_image.getSampler();
}

const UsageType& Texture::getUsage() const
{
   return m_usage;
}

void Texture::destroy()
{
   m_image.destroy();
}

Texture::~Texture() {}
