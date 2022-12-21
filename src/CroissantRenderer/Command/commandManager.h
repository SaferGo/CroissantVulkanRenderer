#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Pipeline/Pipeline.h>

namespace commandManager
{
   namespace action
   {
      void copyBufferToBuffer(
            const VkBuffer& srcBuffer,
            const VkBuffer& dstBuffer,
            const uint32_t& regionCount,
            const VkBufferCopy& regions,
            const VkCommandBuffer& commandBuffer
      );
      void copyBufferToImage(
            const VkBuffer& srcBuffer,
            const VkImage& dstImage,
            const VkImageLayout& dstImageLayout,
            const uint32_t& regionCount,
            const VkBufferImageCopy& regions,
            const VkCommandBuffer& commandBuffer
      );

      void drawIndexed(
            const uint32_t& indexCount,
            const uint32_t& instanceCount,
            const uint32_t& firstIndex,
            const uint32_t& vertexOffset,
            const uint32_t& firstInstance,
            const VkCommandBuffer& commandBuffer
      );

      void dispatch(
            const uint32_t& xSize,
            const uint32_t& ySize,
            const uint32_t& zSize,
            const VkCommandBuffer& commandBuffer
      );
   };

   namespace state
   {
      void bindPipeline(
            const VkPipeline& pipeline,
            const PipelineType& pipelineType,
            const VkCommandBuffer& commandBuffer
      );

      void bindVertexBuffers(
            const std::vector<VkBuffer>& vertexBuffers,
            const std::vector<VkDeviceSize>& offsets,
            const uint32_t& indexOfFirstBinding,
            const uint32_t& bindingCount,
            const VkCommandBuffer& commandBuffer
      );
      void bindIndexBuffer(
            const VkBuffer& indexBuffer,
            const VkDeviceSize& offset,
            const VkIndexType& indexType,
            const VkCommandBuffer& commandBuffer
      );

      void bindDescriptorSets(
            const VkPipelineLayout& pipelineLayout,
            const PipelineType& pipelineType,
            const uint32_t& firstSet,
            const std::vector<VkDescriptorSet>& descriptorSets,
            const std::vector<uint32_t>& dynamicOffsets,
            const VkCommandBuffer& commandBuffer
      );

      // TODO -> setViewportS
      void setViewport(
            const float& x,
            const float& y,
            const VkExtent2D& extent,
            const float& minDepth,
            const float& maxDepth,
            const uint32_t& firstViewport,
            const uint32_t& viewportCount,
            const VkCommandBuffer& commandBuffer
      );

      // TODO -> setScissorS
      void setScissor(
            const VkOffset2D& offset,
            const VkExtent2D& extent,
            const uint32_t& firstScissor,
            const uint32_t& scissorCount,
            const VkCommandBuffer& commandBuffer
      );
   };

   namespace synchronization
   {
      void recordPipelineBarrier(
            const VkPipelineStageFlags& srcStageFlags,
            const VkPipelineStageFlags& dstStageFlags,
            const VkDependencyFlags& dependencyFlags,
            const VkCommandBuffer& commandBuffer,
            const std::vector<VkMemoryBarrier>& memoryBarriers,
            const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers,
            const std::vector<VkImageMemoryBarrier>& imageMemoryBarriers
      );
   };
};
