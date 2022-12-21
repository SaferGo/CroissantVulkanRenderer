#pragma once

#include <vector>
#include <memory>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <CroissantRenderer/Window/Window.h>
#include <CroissantRenderer/GUI/GUI.h>
#include <CroissantRenderer/Queue/QueueFamilyIndices.h>
#include <CroissantRenderer/Queue/QueueFamilyHandles.h>
#include <CroissantRenderer/Swapchain/Swapchain.h>
#include <CroissantRenderer/Computation/Computation.h>
#include <CroissantRenderer/Pipeline/Graphics.h>
#include <CroissantRenderer/Pipeline/Compute.h>
#include <CroissantRenderer/Features/DepthBuffer.h>
#include <CroissantRenderer/RenderPass/RenderPass.h>
#include <CroissantRenderer/Command/CommandPool.h>
#include <CroissantRenderer/Device/Device.h>
#include <CroissantRenderer/Descriptor/DescriptorPool.h>
#include <CroissantRenderer/Texture/Texture.h>
#include <CroissantRenderer/Model/ModelInfo.h>
#include <CroissantRenderer/Model/Model.h>
#include <CroissantRenderer/Model/Types/Skybox.h>
#include <CroissantRenderer/Model/Types/Light.h>
#include <CroissantRenderer/Camera/Camera.h>
#include <CroissantRenderer/Camera/Types/Arcball.h>
#include <CroissantRenderer/Features/ShadowMap.h>
#include <CroissantRenderer/VKinstance/VKinstance.h>
#include <CroissantRenderer/Scene/Scene.h>

class Renderer
{

public:

   void run();
   void addObjectPBR(
         const std::string& name,
         const std::string& folderName,
         const std::string& fileName,
         const glm::fvec3& pos = glm::fvec4(0.0f),
         const glm::fvec3& rot = glm::fvec3(0.0f),
         const glm::fvec3& size = glm::fvec3(1.0f)
   );
   void addDirectionalLight(
         const std::string& name,
         const std::string& folderName,
         const std::string& fileName,
         const glm::fvec3& color,
         const glm::fvec3& pos,
         const glm::fvec3& endPos,
         const glm::fvec3& size
   );
   void addSpotLight(
         const std::string& name,
         const std::string& folderName,
         const std::string& fileName,
         const glm::fvec3& color,
         const glm::fvec3& pos,
         const glm::fvec3& endPos,
         const glm::fvec3& rot,
         const glm::fvec3& size
   );
   void addPointLight(
         const std::string& name,
         const std::string& folderName,
         const std::string& fileName,
         const glm::fvec3& color,
         const glm::fvec3& pos,
         const glm::fvec3& size
   );
   void addSkybox(
         const std::string& fileName,
         const std::string& textureFolderName
   );

private:

   void createCommandPools();
   void initVK();
   void doComputations();
   void handleInput();
   void calculateFrames(double& lastTime, int& framesCounter);
   static void scrollCallback(
         GLFWwindow* window,
         double xoffset,
         double yoffset
   );
   void mainLoop();
   void cleanup();
   void configureUserInputs();
   void recordCommandBuffer(
         const VkFramebuffer& framebuffer,
         const RenderPass& renderPass,
         const VkExtent2D& extent,
         const std::vector<const Graphics*>& graphicsPipelines,
         const uint32_t currentFrame,
         const VkCommandBuffer& commandBuffer,
         const std::vector<VkClearValue>& clearValues,
         const std::shared_ptr<CommandPool>& commandPool
   );
   void drawFrame(uint8_t& currentFrame);

   void createSyncObjects();
   void destroySyncObjects();

   std::shared_ptr<Window>             m_window;
   std::unique_ptr<GUI>                m_GUI;
   std::shared_ptr<Camera>             m_camera;

   std::unique_ptr<VKinstance>         m_vkInstance;
   std::unique_ptr<Device>             m_device;
   QueueFamilyIndices                  m_qfIndices;
   QueueFamilyHandles                  m_qfHandles;

   std::shared_ptr<Swapchain>          m_swapchain;

   Scene                               m_scene;

   std::vector<VkSemaphore>            m_imageAvailableSemaphores;
   std::vector<VkSemaphore>            m_renderFinishedSemaphores;
   std::vector<VkFence>                m_inFlightFences;

   std::vector<ModelInfo>              m_modelsToLoadInfo;

   // Command Pool for main drawing commands.
   std::shared_ptr<CommandPool>        m_commandPoolForGraphics;
   std::shared_ptr<CommandPool>        m_commandPoolForCompute;

   DescriptorPool                      m_descriptorPoolForGraphics;
   DescriptorPool                      m_descriptorPoolForComputations;

   // NUMBER OF VK_ATTACHMENT_LOAD_OP_CLEAR == CLEAR_VALUES
   std::vector<VkClearValue> m_clearValues;
   std::vector<VkClearValue> m_clearValuesShadowMap;
   bool m_isMouseInMotion;

   // milliseconds per frame
   double m_mpf;

   //---------------------------Features--------------------------------------
   DepthBuffer                                         m_depthBuffer;
   MSAA                                                m_msaa;
   std::shared_ptr<ShadowMap<Attributes::PBR::Vertex>> m_shadowMap;

};
