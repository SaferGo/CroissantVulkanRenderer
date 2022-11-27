#include <VulkanToyRenderer/Textures/mipmapUtils.h>

#include <cmath>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Commands/commandManager.h>

void mipmapUtils::generateMipmaps(
   const VkPhysicalDevice& physicalDevice,
   const std::shared_ptr<CommandPool>& commandPool,
   const VkQueue& graphicsQueue,
   const VkImage& image,
   const int32_t width,
   const int32_t height,
   const VkFormat& format,
   const int32_t mipLevels
) {

   if (!isLinearBlittingSupported(physicalDevice, format))
      throw std::runtime_error(
            "Texture image format does not support linear blitting.\n"
      );

   VkCommandBuffer commandBuffer;

   commandPool->allocCommandBuffer(commandBuffer, true);

   commandPool->beginCommandBuffer(
         VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
         commandBuffer
   );

      VkImageMemoryBarrier imgMemoryBarrier{};
      imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      imgMemoryBarrier.image = image;
      imgMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      imgMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      imgMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imgMemoryBarrier.subresourceRange.baseArrayLayer = 0;
      imgMemoryBarrier.subresourceRange.layerCount = 1;
      imgMemoryBarrier.subresourceRange.levelCount = 1;

      
      int32_t mipWidth = width;
      int32_t mipHeight = height;

      for (int32_t i = 1; i < mipLevels; i++)
      {
         imgMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
         imgMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
         imgMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
         imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

         commandManager::synchronization::recordPipelineBarrier(
               VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_PIPELINE_STAGE_TRANSFER_BIT,
               0,
               commandBuffer,
               {},
               {},
               {imgMemoryBarrier}
         );

         VkImageBlit blit{};
         blit.srcOffsets[0] = {0, 0, 0};
         blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
         blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
         blit.srcSubresource.mipLevel = i - 1;
         blit.srcSubresource.baseArrayLayer = 0;
         blit.srcSubresource.layerCount = 1;
         blit.dstOffsets[0] = {0, 0, 0};
         blit.dstOffsets[1] = {
            mipWidth > 1 ? mipWidth / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            1
         };
         blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
         blit.dstSubresource.mipLevel = i;
         blit.dstSubresource.baseArrayLayer = 0;
         blit.dstSubresource.layerCount = 1;

         vkCmdBlitImage(
               commandBuffer,
               image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
               image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               1, &blit,
               VK_FILTER_LINEAR
         );

         imgMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
         imgMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
         imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

         commandManager::synchronization::recordPipelineBarrier(
               VK_PIPELINE_STAGE_TRANSFER_BIT,
               VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
               0,
               commandBuffer,
               {},
               {},
               {imgMemoryBarrier}
         );

         if (mipWidth > 1)
            mipWidth /= 2;
         if (mipHeight > 1)
            mipHeight /= 2;
      }

      imgMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
      imgMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      imgMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

      commandManager::synchronization::recordPipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
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


bool mipmapUtils::isLinearBlittingSupported(
      const VkPhysicalDevice& physicalDevice,
      const VkFormat& format
) {
   VkFormatProperties formatProperties;
   vkGetPhysicalDeviceFormatProperties(
         physicalDevice,
         format,
         &formatProperties
   );

   if (!(formatProperties.optimalTilingFeatures &
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
      )
   ) {
      return false;
   }

   return true;
}

const int32_t mipmapUtils::getAmountOfSupportedMipLevels(
      const int32_t width,
      const int32_t height
) {
   return std::floor(
         std::log2(
            std::max(
               width,
               height
            )
         )
   ) + 1;
}
