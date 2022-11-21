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
#include <VulkanToyRenderer/Features/DepthBuffer.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Device/Device.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Model/Types/Skybox.h>
#include <VulkanToyRenderer/Model/Types/Light.h>
#include <VulkanToyRenderer/Camera/Camera.h>
#include <VulkanToyRenderer/Camera/Types/Arcball.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>


struct ModelToLoadInfo
{
   ModelType   type;
   std::string name;
   std::string modelFileName;
   glm::fvec3  color;
   glm::fvec3  pos;
   glm::fvec3  rot;
   glm::fvec3  size;

   // For light models.
   LightType   lType;
   glm::fvec3  endPos;
   const float attenuation;
   const float radius;
};

class Renderer
{

public:

   void run();
   void loadModel(const size_t startI, const size_t chunckSize);
   void loadModels();
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
         const glm::fvec3& pos,
         const glm::fvec3& endPos,
         const glm::fvec3& size
   );
   void addSpotLight(
         const std::string& name,
         const std::string& modelFileName,
         const glm::fvec3& color,
         const glm::fvec3& pos,
         const glm::fvec3& endPos,
         const glm::fvec3& rot,
         const glm::fvec3& size,
         const float attenuation,
         const float radius
   );
   void addPointLight(
         const std::string& name,
         const std::string& modelFileName,
         const glm::fvec3& color,
         const glm::fvec3& pos,
         const glm::fvec3& size,
         const float attenuation,
         const float radius
   );
   void addSkybox(
         const std::string& name,
         const std::string& textureFolderName
   );

private:

   void createGraphicsPipelines();
   void uploadModels();
   void initVK();
   void handleInput();
   static void scrollCallback(
         GLFWwindow* window,
         double xoffset,
         double yoffset
   );
   void mainLoop();
   void cleanup();
   void createShadowMapRenderPass();
   void createSceneRenderPass();
   void createDescriptorSetLayouts();
   void recordCommandBuffer(
         const VkFramebuffer& framebuffer,
         const RenderPass& renderPass,
         const VkExtent2D& extent,
         const std::vector<GraphicsPipeline>& graphicsPipelines,
         const uint32_t currentFrame,
         const VkCommandBuffer& commandBuffer,
         const std::vector<VkClearValue>& clearValues,
         const std::shared_ptr<CommandPool>& commandPool
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

   std::shared_ptr<Window>             m_window;
   std::unique_ptr<GUI>                m_GUI;
   std::shared_ptr<Camera>             m_camera;

   VkInstance                          m_vkInstance;
   Device                              m_device;
   VkDebugUtilsMessengerEXT            m_debugMessenger;
   QueueFamilyIndices                  m_qfIndices;
   QueueFamilyHandles                  m_qfHandles;

   std::unique_ptr<Swapchain>          m_swapchain;
   RenderPass                          m_renderPass;
   RenderPass                          m_renderPassShadowMap;

   std::vector<VkSemaphore>            m_imageAvailableSemaphores;
   std::vector<VkSemaphore>            m_renderFinishedSemaphores;
   std::vector<VkFence>                m_inFlightFences;

   std::vector<ModelToLoadInfo>        m_modelsToLoadInfo;
   std::vector<std::shared_ptr<Model>> m_models;
   std::vector<size_t>                 m_objectModelIndices;
   std::vector<size_t>                 m_lightModelIndices;
   std::vector<size_t>                 m_skyboxModelIndex;
   std::optional<size_t>               m_directionalLightIndex;
   std::shared_ptr<Skybox>             m_skybox;

   // TODO: delete this
   size_t                  m_mainModelIndex;

   GraphicsPipeline                    m_graphicsPipelinePBR;
   GraphicsPipeline                    m_graphicsPipelineSkybox;
   GraphicsPipeline                    m_graphicsPipelineLight;
   GraphicsPipeline                    m_graphicsPipelineShadowMap;

   // Command buffer for main drawing commands.
   std::shared_ptr<CommandPool>        m_commandPool;

   DescriptorPool                      m_descriptorPool;
   VkDescriptorSetLayout               m_descriptorSetLayoutNormalPBR;
   VkDescriptorSetLayout               m_descriptorSetLayoutLight;
   VkDescriptorSetLayout               m_descriptorSetLayoutSkybox;
   VkDescriptorSetLayout               m_descriptorSetLayoutShadowMap;

   // NUMBER OF VK_ATTACHMENT_LOAD_OP_CLEAR == CLEAR_VALUES
   std::vector<VkClearValue> m_clearValues;
   std::vector<VkClearValue> m_clearValuesShadowMap;
   bool m_isMouseInMotion;

   //---------------------------Features--------------------------------------
   DepthBuffer  m_depthBuffer;
   MSAA         m_msaa;
   std::shared_ptr<ShadowMap> m_shadowMap;
   std::shared_ptr<Texture>   m_irradianceMap;

};
