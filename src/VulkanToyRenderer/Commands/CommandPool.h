#pragma once

#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>

class CommandPool
{

public:

   CommandPool();
   CommandPool(
         const VkDevice& logicalDevice,
         const VkCommandPoolCreateFlags& flags,
         const uint32_t& graphicsFamilyIndex
   );
   ~CommandPool();
   const VkCommandPool& get() const;
   void beginCommandBuffer(
      const VkCommandBufferUsageFlags& flags,
      const VkCommandBuffer& commandBuffer
   );
   void beginCommandBuffer(
         const VkCommandBufferUsageFlags& flags,
         const uint32_t& cmdBufferIndex
   );
   void endCommandBuffer(const VkCommandBuffer& commandBuffer);
   void destroy();
   void allocCommandBuffer(
         VkCommandBuffer& commandBuffer,
         const bool isOneTimeUsage
   );
   void allocCommandBuffers(const uint32_t& commandBuffersCount);
   void submitCommandBuffer(
         const VkQueue& graphicsQueue,
         const VkCommandBuffer& commandBuffer
   );
   const VkCommandBuffer& getCommandBuffer(const uint32_t index) const;
   void resetCommandBuffer(const uint32_t index);

   void freeCommandBuffer(VkCommandBuffer& commandBuffer);

private:

   void createCommandBufferAllocateInfo(
         const uint32_t& commandBuffersCount,
         VkCommandBufferAllocateInfo& allocInfo
   );

   VkCommandPool m_commandPool;
   VkDevice m_logicalDevice;
   std::vector<VkCommandBuffer> m_commandBuffers;
};
