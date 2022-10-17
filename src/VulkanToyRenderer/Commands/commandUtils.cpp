#include <VulkanToyRenderer/Commands/commandUtils.h>

#include <vector>

//////////////////////////////////ACTION CMDs//////////////////////////////////

void commandUtils::ACTION::copyBufferToImage(
      const VkBuffer& srcBuffer,
      const VkImage& dstImage,
      const VkImageLayout& dstImageLayout,
      const uint32_t& regionCount,
      const VkBufferImageCopy& regions,
      const VkCommandBuffer& commandBuffer
) {
   vkCmdCopyBufferToImage(
         commandBuffer,
         srcBuffer,
         dstImage,
         // Indicates wich layout the image is currently using. Here
         // we are asumming that the image has already been transitioned to
         // the layout that is optimal for copying pixels to.
         dstImageLayout,
         // Count of VkBufferImageCopy(it can be an array of that).
         regionCount,
         &regions
   );
}

void commandUtils::ACTION::copyBufferToBuffer(
      const VkBuffer& srcBuffer,
      const VkBuffer& dstBuffer,
      const uint32_t& regionCount,
      const VkBufferCopy& regions,
      const VkCommandBuffer& commandBuffer
) {
   vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, &regions);
}


void commandUtils::ACTION::drawIndexed(
      const uint32_t& indexCount,
      const uint32_t& instanceCount,
      const uint32_t& firstIndex,
      const uint32_t& vertexOffset,
      const uint32_t& firstInstance,
      const VkCommandBuffer& commandBuffer
) {
   vkCmdDrawIndexed(
         commandBuffer,
         indexCount,
         instanceCount,
         firstIndex,
         vertexOffset,
         firstInstance
   );
}

//////////////////////////////////////STATE////////////////////////////////////

void commandUtils::STATE::bindPipeline(
      const VkPipeline& pipeline,
      const VkCommandBuffer& commandBuffer
) {
   vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void commandUtils::STATE::bindVertexBuffers(
      const std::vector<VkBuffer>& vertexBuffers,
      const std::vector<VkDeviceSize>& offsets,
      const uint32_t& indexOfFirstBinding,
      const uint32_t& bindingCount,
      const VkCommandBuffer& commandBuffer
) {
   vkCmdBindVertexBuffers(
         commandBuffer,
         indexOfFirstBinding,
         bindingCount,
         vertexBuffers.data(),
         offsets.data()
   );
}

void commandUtils::STATE::bindIndexBuffer(
      const VkBuffer& indexBuffer,
      const VkDeviceSize& offset,
      const VkIndexType& indexType,
      const VkCommandBuffer& commandBuffer
) {

   vkCmdBindIndexBuffer(
         commandBuffer,
         indexBuffer,
         0,
         VK_INDEX_TYPE_UINT32
   );
}

void commandUtils::STATE::setViewport(
      const float& x,
      const float& y,
      const VkExtent2D& extent,
      const float& minDepth,
      const float& maxDepth,
      const uint32_t& firstViewport,
      const uint32_t& viewportCount,
      const VkCommandBuffer& commandBuffer
) {
   VkViewport viewport{};
   viewport.x = x;
   viewport.y = y;
   viewport.width = static_cast<float>(extent.width);
   viewport.height = static_cast<float>(extent.height);
   viewport.minDepth = minDepth;
   viewport.maxDepth = maxDepth;

   vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, &viewport);
}

void commandUtils::STATE::setScissor(
      const VkOffset2D& offset,
      const VkExtent2D& extent,
      const uint32_t& firstScissor,
      const uint32_t& scissorCount,
      const VkCommandBuffer& commandBuffer
) {
   VkRect2D scissor{};
   scissor.offset = offset;
   scissor.extent = extent;

   vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, &scissor);
}

void commandUtils::STATE::bindDescriptorSets(
      const VkPipelineLayout& pipelineLayout,
      const uint32_t& firstSet,
      const std::vector<VkDescriptorSet>& descriptorSets,
      const std::vector<uint32_t>& dynamicOffsets,
      const VkCommandBuffer& commandBuffer
) {
   vkCmdBindDescriptorSets(
         commandBuffer,
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         pipelineLayout,
         // Index of the first descriptor set.
         firstSet,
         descriptorSets.size(),
         descriptorSets.data(),
         dynamicOffsets.size(),
         dynamicOffsets.data()
   );
}


////////////////////////////////////Sync. CMDs/////////////////////////////////


void commandUtils::SYNCHRONIZATION::recordPipelineBarrier(
      const VkPipelineStageFlags& srcStageFlags,
      const VkPipelineStageFlags& dstStageFlags,
      const VkDependencyFlags& dependencyFlags,
      const uint32_t& memoryBarrierCount,
      const VkMemoryBarrier* memoryBarriers,
      const uint32_t& bufferMemoryBarrierCount,
      const VkBufferMemoryBarrier* bufferMemoryBarriers,
      const uint32_t& imageMemoryBarrierCount,
      const VkImageMemoryBarrier* imageMemoryBarriers,
      const VkCommandBuffer& commandBuffer
) {

   vkCmdPipelineBarrier(
         commandBuffer,
         // Pipeline stage in which operations occur that should happen 
         // before the barrier.
         srcStageFlags,
         // Pipeline stage in which operations will wait on the barrier.
         dstStageFlags,
         // 0 or VK_DEPENDENCY_BY_REGION_BIT(per-region condition)
         dependencyFlags,
         // References arrays of pipeline barries of the three available
         // types: memory barriers, buffer memory barriers, and image memory
         // barriers.
         memoryBarrierCount, memoryBarriers,
         bufferMemoryBarrierCount, bufferMemoryBarriers,
         imageMemoryBarrierCount, imageMemoryBarriers
   );
}
