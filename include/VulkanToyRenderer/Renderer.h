#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Window/Window.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyHandles.h>
#include <VulkanToyRenderer/Swapchain/Swapchain.h>
#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipelineManager.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Device/Device.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Textures/Texture.h>

class Renderer
{

public:

   void run();

private:

   void initVK();
   void mainLoop();
   void cleanup();
   void drawFrame(uint8_t& currentFrame);

   void createVkInstance();
   void createSyncObjects();

   void destroySyncObjects();

   Window                   m_window;
   VkInstance               m_vkInstance;
   Device                   m_device;
   QueueFamilyIndices       m_qfIndices;
   QueueFamilyHandles       m_qfHandles;
   Swapchain                m_swapchain;
   RenderPass               m_renderPass;
   GraphicsPipelineManager  m_graphicsPipelineM;
   VkDebugUtilsMessengerEXT m_debugMessenger;
   std::vector<CommandPool> m_commandPools;
   DescriptorPool           m_descriptorPool;
   // Future improv.
   //CommandPool              m_commandPoolMemoryAlloc;

   // Sync objects(for each frame)
   std::vector<VkSemaphore> m_imageAvailableSemaphores;
   std::vector<VkSemaphore> m_renderFinishedSemaphores;
   std::vector<VkFence>     m_inFlightFences;

   // Buffers with their memories
   VkBuffer m_vertexBuffer;
   VkDeviceMemory m_memory1;

   VkBuffer m_indexBuffer;
   VkDeviceMemory m_memory2;

   // Textures
   Texture m_texture;
};
