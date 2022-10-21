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
#include <VulkanToyRenderer/Camera/Camera.h>
#include <VulkanToyRenderer/Camera/Types/Arcball.h>

class Renderer
{

public:

   void run();
   void addObjectPBR(
         const std::string& name,
         const std::string& modelFileName,
         const glm::fvec3& pos = glm::fvec4(0.0f),
         const glm::fvec3& rot = glm::fvec3(0.0f),
         const glm::fvec3& size = glm::fvec3(1.0f)
   );
   void addDirectionalLight(
         const std::string& name,
         const std::string& modelFileName,
         const glm::fvec3& color,
         const glm::fvec3& pos = glm::fvec4(0.0f),
         const glm::fvec3& rot = glm::fvec3(0.0f),
         const glm::fvec3& size = glm::fvec3(1.0f)
   );
   void addSkybox(
         const std::string& name,
         const std::string& textureFolderName
   );

private:

   void createGraphicsPipelines();
   void uploadAllData();
   void initVK();
   void handleInput();
   static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
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

   std::vector<std::shared_ptr<Model>> m_allModels;
   // Models that interact with the light.
   std::vector<size_t> m_normalModelIndices;
   std::vector<size_t> m_skyboxModelIndices;
   std::vector<size_t> m_directionalLightIndices;

   
   DescriptorPool             m_descriptorPool;
   GraphicsPipeline           m_graphicsPipelinePBR;
   GraphicsPipeline           m_graphicsPipelineSkybox;
   GraphicsPipeline           m_graphicsPipelineDirectionalLight;
   VkDescriptorSetLayout      m_descriptorSetLayoutNormalPBR;
   VkDescriptorSetLayout      m_descriptorSetLayoutDirectionalLight;
   VkDescriptorSetLayout      m_descriptorSetLayoutSkybox;

   // NUMBER OF VK_ATTACHMENT_LOAD_OP_CLEAR == CLEAR_VALUES
   std::vector<VkClearValue> m_clearValues;
   std::shared_ptr<Camera> m_camera;
   bool m_isMouseInMotion;
};
