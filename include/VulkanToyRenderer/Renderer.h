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
#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipeline.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Device/Device.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Model/Types/Skybox.h>

class Renderer
{

public:

   void run();
   void addObjectPBR(
         const std::string& name,
         const std::string& modelFileName
   );
   void addSkybox(
         const std::string& name,
         const std::string& textureFolderName
   );

private:

   void createGraphicsPipelines();
   void uploadAllData();
   template<typename T>
   void updateUniformBuffer(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent,
         const std::shared_ptr<T>& model
   );
   glm::mat4 getUpdatedModelMatrix(
         const glm::fvec4 actualPos,
         const glm::fvec3 actualRot,
         const glm::fvec3 actualSize
   );
   glm::mat4 getUpdatedViewMatrix(
         const glm::fvec3& cameraPos,
         const glm::fvec3& centerPos,
         const glm::fvec3& upAxis
   );
   glm::mat4 getUpdatedProjMatrix(
      const float vfov,
      const float aspect,
      const float nearZ,
      const float farZ
   );

   void initVK();
   void mainLoop();
   void cleanup();
   void createRenderPass();
   void createDescriptorSetLayouts();
   void recordCommandBuffer(
         const VkFramebuffer& framebuffer,
         const RenderPass& renderPass,
         const VkExtent2D& extent,
         const std::vector<GraphicsPipeline>& graphicsPipelines,
         const uint32_t currentFrame,
         const VkCommandBuffer& commandBuffer,
         CommandPool& commandPool
   );
   template <typename T>
   void bindAllMeshesData(
         const std::shared_ptr<T>& model,
         const GraphicsPipeline& graphicsPipeline,
         const VkCommandBuffer& commandBuffer,
         const uint32_t currentFrame
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
   std::vector<size_t> m_skyboxModelIndices;

   
   DescriptorPool             m_descriptorPool;
   GraphicsPipeline           m_graphicsPipelinePBR;
   GraphicsPipeline           m_graphicsPipelineSkybox;
   GraphicsPipeline           m_graphicsPipelineLightM;
   VkDescriptorSetLayout      m_descriptorSetLayoutNormalPBR;
   VkDescriptorSetLayout      m_descriptorSetLayoutLight;
   VkDescriptorSetLayout      m_descriptorSetLayoutSkybox;

   // NUMBER OF VK_ATTACHMENT_LOAD_OP_CLEAR == CLEAR_VALUES
   std::vector<VkClearValue> m_clearValues;
   glm::fvec4 m_cameraPos;
};
