#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>

class CommandPool
{

public:

   CommandPool(
      const VkDevice& logicalDevice,
      QueueFamilyIndices& queueFamilyIndices
   );
   ~CommandPool();
   void destroyCommandPool();
   void allocCommandBuffer(VkCommandBuffer& commandBuffer);
   void allocAllCommandBuffers();
   const VkCommandBuffer& getCommandBuffer(const uint32_t index);
   void resetCommandBuffer(const uint32_t index);
   void resetCommandBuffer(VkCommandBuffer& commandBuffer);
   void freeCommandBuffer(VkCommandBuffer& commandBuffer);
   void recordCommandBuffer(
      const VkFramebuffer& framebuffer,
      const VkRenderPass& renderPass,
      const VkExtent2D& extent,
      const VkPipeline& graphicsPipeline,
      const uint32_t index,
      const VkBuffer& vertexBuffer,
      const VkBuffer& indexBuffer,
      const size_t vertexCount,
      VkPipelineLayout& pipelineLayout,
      const std::vector<VkDescriptorSet>& descriptorSets
   );

private:

   //-----------------Methods needed to record cmd buffer----------------------
   void createRenderPassBeginInfo(
         const VkRenderPass& renderPass,
         const VkFramebuffer& framebuffer,
         const VkExtent2D& extent,
         VkRenderPassBeginInfo& renderPassInfo
   );
   void bindVertexBuffers(
         const VkBuffer& vertexBuffer,
         VkCommandBuffer& commandBuffer
   );
   void bindIndexBuffer(
      const VkBuffer& indexBuffer,
      VkCommandBuffer& commandBuffer
   );
   void setViewport(
         const VkExtent2D& extent,
         VkCommandBuffer& commandBuffer
   );
   void setScissor(
         const VkExtent2D& extent,
         VkCommandBuffer& commandBuffer
   );
   void createCommandBufferBeginInfo(
      VkCommandBuffer& commandBuffer,
      VkCommandBufferBeginInfo &beginInfo
   );
   //--------------------------------------------------------------------------

   VkCommandPool m_commandPool;
   VkDevice m_logicalDevice;
   QueueFamilyIndices m_queueFamilyIndices;
   // For now it will be custom. We'll just have 2.
   std::vector<VkCommandBuffer> m_commandBuffers;
};
