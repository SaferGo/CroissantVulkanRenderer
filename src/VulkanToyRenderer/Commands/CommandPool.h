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
   void beginCommandBuffer(
      const VkCommandBufferUsageFlags& flags,
      VkCommandBuffer& commandBuffer
   );
   void endCommandBuffer(VkCommandBuffer& commandBuffer);
   void destroyCommandPool();
   void allocCommandBuffer(VkCommandBuffer& commandBuffer);
   void submitCommandBuffer(
         VkQueue& graphicsQueue,
         VkCommandBuffer& commandBuffer
   );
   void allocAllCommandBuffers();
   const VkCommandBuffer& getCommandBuffer(const uint32_t index);
   void resetCommandBuffer(const uint32_t index);
   void recordCommandBuffer(
      const VkFramebuffer& framebuffer,
      const VkRenderPass& renderPass,
      const VkExtent2D& extent,
      const VkPipeline& graphicsPipeline,
      const uint32_t index,
      const VkBuffer& vertexBuffer,
      const VkBuffer& indexBuffer,
      const size_t indexCount,
      const VkPipelineLayout& pipelineLayout,
      const std::vector<VkDescriptorSet>& descriptorSets
   );

private:

   void freeCommandBuffer(VkCommandBuffer& commandBuffer);

   void createRenderPassBeginInfo(
         const VkRenderPass& renderPass,
         const VkFramebuffer& framebuffer,
         const VkExtent2D& extent,
         const std::vector<VkClearValue>& clearValues,
         VkRenderPassBeginInfo& renderPassInfo
   );

   //--------------------------------------------------------------------------

   VkCommandPool m_commandPool;
   VkDevice m_logicalDevice;
   QueueFamilyIndices m_queueFamilyIndices;
   // For now it will be custom. We'll just have 2.
   std::vector<VkCommandBuffer> m_commandBuffers;
};
