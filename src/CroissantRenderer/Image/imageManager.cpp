#include <CroissantRenderer/Image/imageManager.h>

#include <stdexcept>

#include <CroissantRenderer/Command/commandManager.h>
#include <CroissantRenderer/Command/CommandPool.h>
#include <CroissantRenderer/Buffer/bufferUtils.h>
#include <CroissantRenderer/Buffer/bufferManager.h>

void imageManager::createImage(
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
      VkImage& image,
      VkDeviceMemory& memory
) {
   // Creates an image object with the array of pixels.
   // (So later we can sample it as texels...so in 2D coords)
   VkImageCreateInfo imageInfo{};
   imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   imageInfo.imageType = VK_IMAGE_TYPE_2D;
   imageInfo.extent.width = width;
   imageInfo.extent.height = height;
   imageInfo.extent.depth = 1;
   imageInfo.mipLevels = mipLevels;

   if (isCubemap)
   {
      imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
      imageInfo.arrayLayers = 6;
   } else
   {
      imageInfo.flags = 0;
      imageInfo.arrayLayers = 1;
   }

   imageInfo.format = format;
   // Specifies how texels are laid out.
   imageInfo.tiling = tiling;
   imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   // The image will be used as destination for the buffer copy, so it should
   // be set up as a transfer destination. Also, we want to be able to access
   // the image from the shader -> VK_IMAGE_USAGE_SAMPLED_BIT
   imageInfo.usage = usage;
   // Only used by one queue family(the graphics q.family because it supports
   // transfer operations).
   imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   // Related to multisampling.
   imageInfo.samples = numSamples;

   auto status = vkCreateImage(
         logicalDevice,
         &imageInfo,
         nullptr,
         &image
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create the Image Object");


   // Allocates memory(in the device) for the image.
   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements(
         logicalDevice,
         image,
         &memRequirements
   );

   VkMemoryAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memRequirements.size;
   allocInfo.memoryTypeIndex = bufferUtils::findMemoryType(
         physicalDevice,
         memRequirements.memoryTypeBits,
         memoryProperties
   );

   status = vkAllocateMemory(
         logicalDevice,
         &allocInfo,
         nullptr,
         &memory
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate image memory!");

   // Bind the image object(it's like a buffer) to the memory.
   vkBindImageMemory(
         logicalDevice,
         image,
         memory,
         0
   );

}

void imageManager::createImageView(
      const VkDevice& logicalDevice,
      const VkFormat& format,
      const VkImage& image,
      const VkImageAspectFlags& aspectFlags,
      const bool isCubemap,
      const uint32_t mipLevels,
      const VkComponentSwizzle& componentMapR,
      const VkComponentSwizzle& componentMapG,
      const VkComponentSwizzle& componentMapB,
      const VkComponentSwizzle& componentMapA,
      VkImageView& imageView
) {

   VkImageViewCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   createInfo.image = image;
   // Specifies how to treat images, as 1D textures, 2D textures, 3D
   // textures and cube maps.
   if (isCubemap)
   {
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
      createInfo.subresourceRange.layerCount = 6;
   } else
   {
      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.subresourceRange.layerCount = 1;
   }

   createInfo.format = format;
   // Specifies how we want to map all the color channels of the images
   // (E.g: map all of the channels to the red channel for a monochrome
   // texture)
   createInfo.components.r = componentMapR;
   createInfo.components.g = componentMapG;
   createInfo.components.b = componentMapB;
   createInfo.components.a = componentMapA;
   // Specifies what the image's purpose is and which part of the image
   // should be accessed.
   // (E.g: with mipmapping leves or multiple layers)
   createInfo.subresourceRange.aspectMask = aspectFlags;
   createInfo.subresourceRange.baseMipLevel = 0;
   createInfo.subresourceRange.levelCount = mipLevels;
   createInfo.subresourceRange.baseArrayLayer = 0;
   
   const auto status = vkCreateImageView(
         logicalDevice,
         &createInfo,
         nullptr,
         &imageView
   );
   
   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create image views!");
 
}

template<typename T>
void imageManager::copyDataToImage(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkDeviceSize size,
      const uint32_t offset,
      T* data,
      const uint32_t width,
      const uint32_t height,
      const VkFormat& format,
      const uint32_t mipLevels,
      const bool isCubemap,
      const VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool,
      const VkImage& image
) {

   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;

   bufferManager::createAndFillStagingBuffer(
         physicalDevice,
         logicalDevice,
         size,
         0,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         (
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
         ),
         stagingBufferMemory,
         stagingBuffer,
         data
   );

   // We will transfer the pixels to the image object with a cmd buffer.
   // (staging buffer to the image obj)
   transitionImageLayout(
         format,
         mipLevels,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         isCubemap,
         commandPool,
         graphicsQueue,
         image
   );


   VkCommandBuffer commandBuffer;

   commandPool->allocCommandBuffer(commandBuffer, true);

   commandPool->beginCommandBuffer(
         VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
         commandBuffer
   );
      // Specifies which part of the buffer is going to be copied to which
      // part of the image.
      VkBufferImageCopy region{};

      // Which parts of the buffer to copy.
      region.bufferOffset = 0;
      region.bufferRowLength = 0;
      region.bufferImageHeight = 0;

      // Which part of the image we want to copy the pixels(from the buffer..).
      region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      region.imageSubresource.mipLevel = 0;
      region.imageSubresource.baseArrayLayer = 0;

      if (isCubemap)
         region.imageSubresource.layerCount = 6;
      else
         region.imageSubresource.layerCount = 1;

      region.imageOffset = {0, 0, 0};
      region.imageExtent = {
         width,
         height,
         1
      };

      commandManager::action::copyBufferToImage(
            stagingBuffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            region,
            commandBuffer
      );


   commandPool->endCommandBuffer(commandBuffer);

   commandPool->submitCommandBuffer(
         graphicsQueue,
         {commandBuffer},
         true
   );

   bufferManager::destroyBuffer(logicalDevice, stagingBuffer);
   bufferManager::freeMemory(logicalDevice, stagingBufferMemory);

   // Another transition to sample from the shader.
   if (isCubemap)
   {
      transitionImageLayout(
            format,
            mipLevels,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            isCubemap,
            commandPool,
            graphicsQueue,
            image
      );
   }
}

/////////////////////////////////Instances/////////////////////////////////////
template void imageManager::copyDataToImage<uint8_t>(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkDeviceSize size,
         const uint32_t offset,
         uint8_t* data,
         const uint32_t width,
         const uint32_t height,
         const VkFormat& format,
         const uint32_t mipLevels,
         const bool isCubemap,
         const VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool,
         const VkImage& image
);
///////////////////////////////////////////////////////////////////////////////

void imageManager::transitionImageLayout(
      const VkFormat& format,
      const uint32_t mipLevels,
      const VkImageLayout& oldLayout,
      const VkImageLayout& newLayout,
      const bool isCubemap,
      const std::shared_ptr<CommandPool>& commandPool,
      const VkQueue& graphicsQueue,
      const VkImage& image
) {
   VkCommandBuffer commandBuffer;

   commandPool->allocCommandBuffer(commandBuffer, true);

   commandPool->beginCommandBuffer(
         VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
         commandBuffer
   );

      VkImageMemoryBarrier imgMemoryBarrier{};
      VkPipelineStageFlags sourceStage, destinationStage;
      createImageMemoryBarrier(
            mipLevels,
            oldLayout,
            newLayout,
            isCubemap,
            image,
            imgMemoryBarrier,
            sourceStage,
            destinationStage
      );

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

void imageManager::createImageMemoryBarrier(
      const uint32_t mipLevels,
      const VkImageLayout& oldLayout,
      const VkImageLayout& newLayout,
      const bool isCubemap,
      const VkImage& image,
      VkImageMemoryBarrier& imgMemoryBarrier,
      VkPipelineStageFlags& sourceStage,
      VkPipelineStageFlags& destinationStage
) {

      imgMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      // We could use VK_IMAGE_LAYOUT_UNDEFINED if we don't care about the
      // existing contents of the image.
      imgMemoryBarrier.oldLayout = oldLayout;
      imgMemoryBarrier.newLayout = newLayout;
      // Since we are not using the barrier to transfer queue family ownership,
      // we'll ignore these two.
      imgMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      imgMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      imgMemoryBarrier.image = image;
      imgMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      imgMemoryBarrier.subresourceRange.baseMipLevel = 0;
      // In this case, our image is not an array(it has 2D coords -> texel).
      imgMemoryBarrier.subresourceRange.baseArrayLayer = 0;

      if (isCubemap)
         imgMemoryBarrier.subresourceRange.layerCount = 6;
      else
         imgMemoryBarrier.subresourceRange.layerCount = 1;

      imgMemoryBarrier.subresourceRange.levelCount = mipLevels;

      //Barriers are primarily used for synchronization purposes, so you must
      //specify which types of operations that involve the resource must happen
      //before the barrier, and which operations that involve the resource must
      //wait on the barrier. We need to do that despite already using
      //vkQueueWaitIdle to manually synchronize. The right values depend on the
      //old and new layout.
      imgMemoryBarrier.srcAccessMask = 0;
      imgMemoryBarrier.dstAccessMask = 0;

      if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
          newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
      ) {

         imgMemoryBarrier.srcAccessMask = 0;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

         sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
         destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

      } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      ) {

         imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

         sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
         destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

      } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      ) {

         imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
         imgMemoryBarrier.dstAccessMask = (
               VK_ACCESS_HOST_WRITE_BIT |
               VK_ACCESS_TRANSFER_WRITE_BIT
         );

         sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
         destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

      } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      ) {

         imgMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

         sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
         destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

      } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                 newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      ) {

         imgMemoryBarrier.srcAccessMask = 0;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

         sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
         destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

      } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
                 newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
      ) {

         imgMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
         imgMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

         sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
         destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

      } else
         throw std::invalid_argument("Unsupported layout transition!");
}


