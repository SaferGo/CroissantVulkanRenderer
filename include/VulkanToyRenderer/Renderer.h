#pragma once

#include <vector>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Window/Window.h>
#include <VulkanToyRenderer/GUI/GUI.h>
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
#include <VulkanToyRenderer/Model/Model.h>

class Renderer
{

public:

   void run();
   void addNormalModel(
         const std::string& name,
         const std::string& meshFile
   );
   void addNormalModel(
         const std::string& name,
         const std::string& meshFile,
         const std::string& textureFile
   );
   void addModel(
         const std::string& name,
         const std::string& meshFile
   );
   void addLightModel(
         const std::string& name,
         const std::string& meshFile
   );
   void addLightModel(
      const std::string& name,
      const std::string& meshFile,
      const std::string& textureFile
   ) ;



private:

   void addModel(
         const std::string& name,
         const std::string& meshFile,
         const std::string& textureFile
   );

   void updateUniformBuffer(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent,
         Model& model
   );

   void initVK();
   void mainLoop();
   void cleanup();
   void createRenderPass();
   void recordCommandBuffer(
         const VkFramebuffer& framebuffer,
         const RenderPass& renderPass,
         const VkExtent2D& extent,
         const VkPipeline& graphicsPipeline,
         const VkPipelineLayout& pipelineLayout,
         const uint32_t currentFrame,
         const VkCommandBuffer& commandBuffer,
         CommandPool& commandPool
   );
   void drawFrame(uint8_t& currentFrame);

   void createVkInstance();
   void createSyncObjects();

   void destroySyncObjects();

   Window                     m_window;
   std::unique_ptr<GUI>       m_GUI;
   VkInstance                 m_vkInstance;
   Device                     m_device;
   QueueFamilyIndices         m_qfIndices;
   QueueFamilyHandles         m_qfHandles;
   std::unique_ptr<Swapchain> m_swapchain;
   RenderPass                 m_renderPass;
   VkDebugUtilsMessengerEXT   m_debugMessenger;
   GraphicsPipelineManager    m_graphicsPipelineM;

   // Command buffer for main drawing commands.
   CommandPool                m_commandPool;

   DepthBuffer                m_depthBuffer;

   // Sync objects(for each frame)
   std::vector<VkSemaphore> m_imageAvailableSemaphores;
   std::vector<VkSemaphore> m_renderFinishedSemaphores;
   std::vector<VkFence>     m_inFlightFences;

   // Models
   std::vector<std::shared_ptr<Model>> m_allModels;
   // The light models will be saved also in m_allModels but we will
   // keep track of them in m_lightModels.
   std::vector<size_t> m_lightModelIndices;
   // Models that interact with the light.
   std::vector<size_t> m_normalModelIndices;

   DescriptorPool             m_descriptorPool;
   VkDescriptorSetLayout m_descriptorSetLayout;

   // NUMBER OF VK_ATTACHMENT_LOAD_OP_CLEAR == CLEAR_VALUES
   std::vector<VkClearValue> m_clearValues;
   glm::fvec3 m_cameraPos;
};
