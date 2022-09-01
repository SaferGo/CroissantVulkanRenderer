#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>

class CommandManager
{

public:

   CommandManager();
   ~CommandManager();
   void createCommandPool(
      const VkDevice& logicalDevice,
      const QueueFamilyIndices& queueFamilyIndices
   );
   void createCommandBuffer(
      const VkDevice& logicalDevice
   );
   void writeCommandIntoCommandBuffer(
      const VkFramebuffer& framebuffer,
      const VkRenderPass& renderPass,
      const VkExtent2D& extent,
      const VkPipeline& graphicsPipeline
   );
   void resetCommandBuffer();
   void destroyCommandPool(const VkDevice& logicalDevice);
   VkCommandBuffer& getCommandBuffer();

private:

   VkCommandPool m_commandPool;
   VkCommandBuffer m_commandBuffer;

};
