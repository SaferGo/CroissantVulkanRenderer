#include <VulkanToyRenderer/Renderer.h>

#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <limits>
#include <algorithm>
#include <chrono>
#include <thread>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifdef RELEASE_MODE_ON
   #include <tracy/Tracy.hpp>
#endif

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Settings/computePipelineConfig.h>
#include <VulkanToyRenderer/Window/Window.h>
#include <VulkanToyRenderer/Queue/QueueFamilyIndices.h>
#include <VulkanToyRenderer/Queue/QueueFamilyHandles.h>
#include <VulkanToyRenderer/Swapchain/Swapchain.h>
#include <VulkanToyRenderer/Pipeline/Graphics.h>
#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Command/commandManager.h>
#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Model/Types/NormalPBR.h>
#include <VulkanToyRenderer/Model/Types/Skybox.h>
#include <VulkanToyRenderer/Model/Types/Light.h>
#include <VulkanToyRenderer/Descriptor/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptor/descriptorSetLayoutManager.h>
#include <VulkanToyRenderer/Descriptor/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptor/Types/UBO/UBOutils.h>
#include <VulkanToyRenderer/Descriptor/Types/UBO/UBOinfo.h>
#include <VulkanToyRenderer/Descriptor/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptor/DescriptorSets.h>
#include <VulkanToyRenderer/Texture/Texture.h>
#include <VulkanToyRenderer/Texture/Type/NormalTexture.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>
#include <VulkanToyRenderer/RenderPass/subPassUtils.h>
#include <VulkanToyRenderer/RenderPass/attachmentUtils.h>
#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/GUI/GUI.h>
#include <VulkanToyRenderer/Camera/Camera.h>
#include <VulkanToyRenderer/Camera/Types/Arcball.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>
#include <VulkanToyRenderer/Image/imageManager.h>


void Renderer::run()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // TODO: Improve this!
   // NUMBER OF VK_ATTACHMENT_LOAD_OP_CLEAR == CLEAR_VALUES
   m_clearValues.resize(2);
   m_clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
   m_clearValues[1].color = {1.0f, 0.0f};

   m_clearValuesShadowMap.resize(2);
   m_clearValuesShadowMap[0].depthStencil.depth = 1.0f;
   m_clearValuesShadowMap[1].depthStencil.depth = 1.0f;
   m_clearValuesShadowMap[0].depthStencil.stencil = 0.0f;
   m_clearValuesShadowMap[1].depthStencil.stencil = 0.0f;

   m_window = std::make_shared<Window>(
         config::RESOLUTION_W,
         config::RESOLUTION_H,
         config::WINDOW_TITLE
   );

   initVK();

   initComputations();
   doComputations();
   loadBRDFlut();

   m_scene.upload(
         m_device->getPhysicalDevice(),
         m_qfHandles.graphicsQueue,
         m_commandPoolGraphics,
         m_descriptorPoolGraphics,
         m_shadowMap,
         m_BRDFlut
   );

   m_camera = std::make_shared<Arcball>(
         m_window->get(),
         glm::fvec4(0.0f, 0.0f, 5.0f, 1.0f),
         glm::fvec4(0.0f),
         config::FOV,
         // Aspect Ratio
         (
            m_swapchain->getExtent().width /
            (float) m_swapchain->getExtent().height
         ),
         config::Z_NEAR,
         config::Z_FAR
   );

   configureUserInputs();

   m_GUI = std::make_unique<GUI>(
         m_device->getPhysicalDevice(),
         m_device->getLogicalDevice(),
         m_vkInstance->get(),
         m_swapchain,
         m_qfIndices.graphicsFamily.value(),
         m_qfHandles.graphicsQueue,
         m_window
   );

   mainLoop();

   cleanup();
}

void Renderer::configureUserInputs()
{
   // Keyword and mouse settings
   m_isMouseInMotion = false;

   // If the user scrolls back, we'll zoom in the image.
   glfwSetWindowUserPointer(m_window->get(), m_camera.get());
   glfwSetScrollCallback(m_window->get(), scrollCallback);
}

void Renderer::addSkybox(
      const std::string& name,
      const std::string& textureFolderName
) {

   m_modelsToLoadInfo.push_back({
         ModelType::SKYBOX,
         name,
         textureFolderName,
         glm::fvec3(0.0f),
         glm::fvec3(0.0f),
         glm::fvec3(0.0f),
         glm::fvec3(0.0f),
         LightType::NONE,
         glm::fvec3(0.0f),
         0.0f,
         0.0f
   });

}

void Renderer::addObjectPBR(
      const std::string& name,
      const std::string& modelFileName,
      const glm::fvec3& pos,
      const glm::fvec3& rot,
      const glm::fvec3& size
) {

   m_modelsToLoadInfo.push_back({
         ModelType::NORMAL_PBR,
         name,
         modelFileName,
         glm::fvec3(0.0f),
         pos,
         rot,
         size,
         LightType::NONE,
         glm::fvec3(0.0f),
         0.0f,
         0.0f
   });

}

void Renderer::addDirectionalLight(
      const std::string& name,
      const std::string& modelFileName,
      const glm::fvec3& color,
      const glm::fvec3& pos,
      const glm::fvec3& endPos,
      const glm::fvec3& size
) {

   m_modelsToLoadInfo.push_back({
         ModelType::LIGHT,
         name,
         modelFileName,
         color,
         pos,
         glm::fvec3(0.0f),
         size,
         LightType::DIRECTIONAL_LIGHT,
         endPos,
         0.0f,
         0.0f
   });
}

void Renderer::addPointLight(
      const std::string& name,
      const std::string& modelFileName,
      const glm::fvec3& color,
      const glm::fvec3& pos,
      const glm::fvec3& size,
      const float attenuation,
      const float radius
) {

   m_modelsToLoadInfo.push_back({
         ModelType::LIGHT,
         name,
         modelFileName,
         color,
         pos,
         glm::fvec3(0.0f),
         size,
         LightType::POINT_LIGHT,
         glm::fvec3(0.0f),
         attenuation,
         radius
   });

}

void Renderer::addSpotLight(
      const std::string& name,
      const std::string& modelFileName,
      const glm::fvec3& color,
      const glm::fvec3& pos,
      const glm::fvec3& endPos,
      const glm::fvec3& rot,
      const glm::fvec3& size,
      const float attenuation,
      const float radius
) {

   m_modelsToLoadInfo.push_back({
         ModelType::LIGHT,
         name,
         modelFileName,
         color,
         pos,
         rot,
         size,
         LightType::SPOT_LIGHT,
         endPos,
         attenuation,
         radius
   });
}

void Renderer::createSyncObjects()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   m_imageAvailableSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
   m_renderFinishedSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
   m_inFlightFences.resize(config::MAX_FRAMES_IN_FLIGHT);

   //---------------------------Sync. Objects Info-----------------------------

   VkSemaphoreCreateInfo semaphoreInfo{};
   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   VkFenceCreateInfo fenceInfo{};
   fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   // Creates the fence in the signaled state, so that the first call to
   // vkWaitForFences() returns immediately since the fence is already signaled.
   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

   //---------------------Creation of Sync. Objects----------------------------

   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
   {
      auto status = vkCreateSemaphore(
            m_device->getLogicalDevice(),
            &semaphoreInfo,
            nullptr,
            &m_imageAvailableSemaphores[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create semaphore!");

      status = vkCreateSemaphore(
         m_device->getLogicalDevice(),
         &semaphoreInfo,
         nullptr,
         &m_renderFinishedSemaphores[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create semaphore!");

      status = vkCreateFence(
            m_device->getLogicalDevice(),
            &fenceInfo,
            nullptr,
            &m_inFlightFences[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create fence!");
   }
}

void Renderer::createShadowMapRenderPass()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // - Attachments

   VkAttachmentDescription shadowMapAttachment{};
   attachmentUtils::createAttachmentDescriptionWithStencil(
         m_depthBuffer.getFormat(),
         VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         shadowMapAttachment
   );


   // Attachment references
   
   VkAttachmentReference shadowMapAttachmentRef{};
   attachmentUtils::createAttachmentReference(
         0,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         shadowMapAttachmentRef
   );

   // Subpasses
   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.flags = 0;
   subpass.pDepthStencilAttachment = &shadowMapAttachmentRef;

   m_renderPassShadowMap = RenderPass(
         m_device->getLogicalDevice(),
         {shadowMapAttachment},
         {subpass},
         {}
         //dependencies
   );

   
}

void Renderer::createPipelines()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // - Features
   {
      m_graphicsPipelineShadowMap = Graphics(
            m_device->getLogicalDevice(),
            GraphicsPipelineType::SHADOWMAP,
            m_swapchain->getExtent(),
            m_renderPassShadowMap.get(),
            {
               {
                  shaderType::VERTEX,
                  "shadowMap"
               }
            },
            VK_SAMPLE_COUNT_1_BIT,
            Attributes::PBR::getBindingDescription(),
            Attributes::PBR::getAttributeDescriptions(),
            m_scene.getObjectModelIndices(),
            GRAPHICS_PIPELINE::SHADOWMAP::UBOS_INFO,
            {}
      );
   }

}

void Renderer::createCommandPools()
{
   // Graphics Command Pool
   {
      m_commandPoolGraphics = std::make_shared<CommandPool>(
            m_device->getLogicalDevice(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            m_qfIndices.graphicsFamily.value()
      );
      
      m_commandPoolGraphics->allocCommandBuffers(
            config::MAX_FRAMES_IN_FLIGHT
      );
   }

   // Compute Command Pool
   {
      m_commandPoolCompute = std::make_shared<CommandPool>(
            m_device->getLogicalDevice(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            m_qfIndices.computeFamily.value()
      );

      m_commandPoolCompute->allocCommandBuffers(1);
   }

   // Features Command Pool
   {
      m_shadowMap->createCommandPool(
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            m_qfIndices.graphicsFamily.value()
      );

      m_shadowMap->allocCommandBuffers(
            config::MAX_FRAMES_IN_FLIGHT
      );
   }

}

void Renderer::initComputations()
{
   m_BRDFcomp = Computation(
         m_device->getPhysicalDevice(),
         m_device->getLogicalDevice(),
         "BRDF",
         sizeof(float),
         2 * sizeof(float) * config::BRDF_HEIGHT * config::BRDF_WIDTH,
         m_qfIndices,
         m_descriptorPoolComputations,
         COMPUTE_PIPELINE::BRDF::BUFFERS_INFO
   );
}

void Renderer::initVK()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   m_vkInstance = std::make_unique<VKinstance>(config::WINDOW_TITLE);

   m_window->createSurface(m_vkInstance->get());

   m_device = std::make_unique<Device>(
         m_vkInstance->get(),
         m_qfIndices,
         m_window->getSurface()
   );
   
   m_qfHandles.setQueueHandles(m_device->getLogicalDevice(), m_qfIndices);

   m_swapchain = std::make_shared<Swapchain>(
         m_device->getPhysicalDevice(),
         m_device->getLogicalDevice(),
         m_window,
         m_device->getSupportedProperties()
   );

   //------------------------------Descriptor Pools----------------------------

   m_descriptorPoolGraphics = DescriptorPool(
         m_device->getLogicalDevice(),
         // Type of descriptors / Count of each type of descriptor in the pool.
         {
            {
               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               // TODO: Make the size more precise.
               1500
               //static_cast<uint32_t> (
               //      m_models.size() * config::MAX_FRAMES_IN_FLIGHT
               //)
            },
            { 
               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               // TODO: Make the size more precise.
               1500
               //static_cast<uint32_t> (
               //      m_models.size() * config::MAX_FRAMES_IN_FLIGHT
               //)
            }
         },
         // Descriptor SETS count.
         // TODO: Make the size more precise.
         1500
         //m_models.size() * config::MAX_FRAMES_IN_FLIGHT
   );

   m_descriptorPoolComputations = DescriptorPool(
         m_device->getLogicalDevice(),
         {
            {
               VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
               2
            }
         },
         1 // just for the BRDF(for now..)
   );

   
   // -------------------------------Main Features-----------------------------

   m_msaa = MSAA(
         m_device->getPhysicalDevice(),
         m_device->getLogicalDevice(),
         m_swapchain->getExtent(),
         m_swapchain->getImageFormat()
   );

   m_depthBuffer = DepthBuffer(
         m_device->getPhysicalDevice(),
         m_device->getLogicalDevice(),
         m_swapchain->getExtent(),
         m_msaa.getSamplesCount()
   );


   m_scene = Scene(
         m_device->getLogicalDevice(),
         m_swapchain->getImageFormat(),
         m_swapchain->getExtent(),
         m_msaa.getSamplesCount(),
         m_depthBuffer.getFormat(),
         m_modelsToLoadInfo
   );
   //----------------------------------RenderPasses----------------------------

   createShadowMapRenderPass();

   //----------------------------------Pipelines-------------------------------

   createPipelines();

   //-----------------------------Secondary Features---------------------------
   //(these features they are not used by all the pipelines and need
   //dependencies)

   m_shadowMap = std::make_shared<ShadowMap<Attributes::PBR::Vertex>>(
         m_device->getPhysicalDevice(),
         m_device->getLogicalDevice(),
         m_swapchain->getExtent().width,
         m_swapchain->getExtent().height,
         m_depthBuffer.getFormat(),
         m_graphicsPipelineShadowMap.getDescriptorSetLayout(),
         config::MAX_FRAMES_IN_FLIGHT,
         &(
          std::dynamic_pointer_cast<NormalPBR>(
             m_scene.getMainModel()
          )->getMeshes()
         )
   );

   //----------------------------------Framebuffers----------------------------

   m_swapchain->createFramebuffers(
         m_scene.getRenderPass(),
         m_depthBuffer,
         m_msaa
   );
   m_shadowMap->createFramebuffer(
         m_renderPassShadowMap,
         m_swapchain->getImageCount()
   );

   //--------------------------------------------------------------------------

   createCommandPools();

   createSyncObjects();

}

void Renderer::recordCommandBuffer(
      const VkFramebuffer& framebuffer,
      const RenderPass& renderPass,
      const VkExtent2D& extent,
      const std::vector<const Graphics*>& graphicsPipelines,
      const uint32_t currentFrame,
      const VkCommandBuffer& commandBuffer,
      const std::vector<VkClearValue>& clearValues,
      const std::shared_ptr<CommandPool>& commandPool
) {

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // Resets the command buffer to be able to be recorded.
   commandPool->resetCommandBuffer(currentFrame);
   // Specifies some details about the usage of this specific command
   // buffer.
   commandPool->beginCommandBuffer(0, commandBuffer);
   
      //--------------------------------RenderPass-----------------------------

      renderPass.begin(
            framebuffer,
            extent,
            clearValues,
            commandBuffer,
            VK_SUBPASS_CONTENTS_INLINE
      );

         //---------------------------------CMDs-------------------------------

         for (auto graphicsPipeline : graphicsPipelines)
         {
            commandManager::state::bindPipeline(
                  graphicsPipeline->get(),
                  PipelineType::GRAPHICS,
                  commandBuffer
            );
            // Set Dynamic States
            commandManager::state::setViewport(
                  0.0f,
                  0.0f,
                  extent,
                  0.0f,
                  1.0f,
                  0,
                  1,
                  commandBuffer
            );
            commandManager::state::setScissor(
                  {0, 0},
                  extent,
                  0,
                  1,
                  commandBuffer
            );

            if (graphicsPipeline->getGraphicsPipelineType() ==
                GraphicsPipelineType::SHADOWMAP
            ) {
               m_shadowMap->bindData(
                     *graphicsPipeline,
                     commandBuffer,
                     currentFrame
               );
               continue;
            }

            // Binds all the models with the same Graphics Pipeline.
            for (auto i : graphicsPipeline->getModelIndices())
            {
               auto& model = m_scene.getModel(i);

               model->bindData(
                     graphicsPipeline,
                     commandBuffer,
                     currentFrame
               );

            }
      }

      renderPass.end(commandBuffer);

   commandPool->endCommandBuffer(commandBuffer);
}

void Renderer::drawFrame(uint8_t& currentFrame)
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // Waits until the previous frame has finished.
   //    - 2 param. -> FenceCount.
   //    - 4 param. -> waitAll.
   //    - 5 param. -> timeOut.
   vkWaitForFences(
         m_device->getLogicalDevice(),
         1,
         &m_inFlightFences[currentFrame],
         VK_TRUE,
         UINT64_MAX
   );

   // After waiting, we need to manually reset the fence.
   vkResetFences(
         m_device->getLogicalDevice(),
         1,
         &m_inFlightFences[currentFrame]
   );

   //------------------------Updates uniform buffer----------------------------

   // First we update the shadow map since the other models of the scene
   // have dependencies with it.

   // Shadow Map
   {
      auto pLight = std::dynamic_pointer_cast<Light>(
            m_scene.getDirectionalLight()
      );

      auto pMainModel = std::dynamic_pointer_cast<NormalPBR>(
            m_scene.getMainModel()
      );
      m_shadowMap->updateUBO(
            // TODO: make it for more than 1 model
            pMainModel->getModelM(),
            pLight->getPos(),
            pLight->getTargetPos(),
            m_camera->getAspect(),
            config::Z_NEAR,
            config::Z_FAR,
            currentFrame
      );
   }

   m_scene.updateUBO(
         m_camera,
         m_shadowMap->getLightSpace(),
         m_swapchain->getExtent(),
         currentFrame
   );


   //--------------------Acquires an image from the swapchain------------------
   
   const uint32_t imageIndex = m_swapchain->getNextImageIndex(
         m_imageAvailableSemaphores[currentFrame]
   );

   //---------------------Records all the command buffer-----------------------

   // Shadow Mapping
   recordCommandBuffer(
         m_shadowMap->getFramebuffer(imageIndex),
         m_renderPassShadowMap,
         m_swapchain->getExtent(),
         {
            &m_graphicsPipelineShadowMap
         },
         currentFrame,
         m_shadowMap->getCommandBuffer(currentFrame),
         m_clearValuesShadowMap,
         m_shadowMap->getCommandPool()
   );
   // Scene
   recordCommandBuffer(
         m_swapchain->getFramebuffer(imageIndex),
         m_scene.getRenderPass(),
         m_swapchain->getExtent(),
         {
            &m_scene.getLightPipeline(),
            &m_scene.getPBRpipeline(),
            // The skybox has to be always the last one.
            &m_scene.getSkyboxPipeline()
         },
         currentFrame,
         m_commandPoolGraphics->getCommandBuffer(currentFrame),
         m_clearValues,
         m_commandPoolGraphics
   );

   // GUI
   m_GUI->recordCommandBuffer(currentFrame,  imageIndex, m_clearValues);

   //----------------------Submits the command buffer--------------------------

   std::vector<VkCommandBuffer> commandBuffersToSubmit = {
      m_shadowMap->getCommandBuffer(currentFrame),
      m_commandPoolGraphics->getCommandBuffer(currentFrame),
      m_GUI->getCommandBuffer(currentFrame)
   };
   std::vector<VkSemaphore> waitSemaphores = {
      m_imageAvailableSemaphores[currentFrame]
   };
   std::vector<VkSemaphore> signalSemaphores = {
      m_renderFinishedSemaphores[currentFrame]
   };

   m_commandPoolGraphics->submitCommandBuffer(
         m_qfHandles.graphicsQueue,
         commandBuffersToSubmit,
         false,
         waitSemaphores,
         (VkPipelineStageFlags)(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
         ),
         signalSemaphores,
         m_inFlightFences[currentFrame]
   );
   

   //-------------------Presentation of the swapchain image--------------------
   
   m_swapchain->presentImage(
         imageIndex,
         signalSemaphores,
         m_qfHandles.presentQueue
   );

   // Updates the frame
   currentFrame = (currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::scrollCallback(
      GLFWwindow* window,
      double xoffset,
      double yoffset
) {

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   Camera* camera = static_cast<Camera*>(
         glfwGetWindowUserPointer(window)
   );

   float actualFOV = camera->getFOV();
   float newFOV = actualFOV + yoffset * -1.0f;

   camera->setFOV(newFOV);
}

void Renderer::handleInput()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   m_window->pollEvents();

   // Avoids any input when we're touching the IMGUI.
   if (m_GUI->isCursorPositionInGUI())
      return;

   if (m_camera->getType() == CameraType::ARCBALL)
   {
      if (glfwGetMouseButton(m_window->get(), GLFW_MOUSE_BUTTON_LEFT) ==
          GLFW_PRESS
      ) {

         auto pCamera = std::dynamic_pointer_cast<Arcball>(m_camera);

         if (!m_isMouseInMotion)
         {
            pCamera->saveCursorPos();

            m_isMouseInMotion = true;

         } else {

            // TODO: Make it dynamic.
            glm::mat4 newRot = glm::mat4(1.0);

            pCamera->updateCameraPos(newRot);
         }

      } else
      {
         m_isMouseInMotion = false;
      }

   }
}

void Renderer::mainLoop()
{
   // Tells us in which frame we are,
   // between 1 <= frame <= MAX_FRAMES_IN_FLIGHT
   uint8_t currentFrame = 0;
   while (m_window->isWindowClosed() == false)
   {
      handleInput();

      // Draws Imgui
      m_GUI->draw(
            m_scene.getModels(),
            m_camera,
            m_scene.getObjectModelIndices(),
            m_scene.getLightModelIndices()
      );
      drawFrame(currentFrame);
   }
   vkDeviceWaitIdle(m_device->getLogicalDevice());
}

void Renderer::loadBRDFlut()
{
   uint32_t bufferSize = (
         2 * sizeof(float) *
         config::BRDF_WIDTH *
         config::BRDF_HEIGHT
   );
   float lutData[bufferSize];

   m_BRDFcomp.downloadData(0, (uint8_t*)lutData, bufferSize);

   gli::texture lutTexture = gli::texture2d(
         gli::FORMAT_RG16_SFLOAT_PACK16,
         gli::extent2d(config::BRDF_WIDTH, config::BRDF_HEIGHT),
         1
   );

   const float* data = lutData;
   for (int y = 0; y < config::BRDF_HEIGHT; y++)
	{
		for (int x = 0; x < config::BRDF_WIDTH; x++)
		{
			const int ofs = y * config::BRDF_HEIGHT + x;
			const gli::vec2 value(data[ofs * 2 + 0], data[ofs * 2 + 1]);
			const gli::texture::extent_type uv = { x, y, 0 };

			lutTexture.store<glm::uint32>(uv, 0, 0, 0, gli::packHalf2x16(value));
		}
	}

   std::string pathToTexture = (
         std::string(SKYBOX_DIR) +
         "Apartment"
         //m_skybox->getTextureFolderName() +
         "/" + 
         "BRDFlut.ktx"
   );

   gli::save_ktx(
         lutTexture,
         pathToTexture
   );

   TextureToLoadInfo info = {
            pathToTexture,
            VK_FORMAT_R16G16_SFLOAT,
            4
   };

   m_BRDFlut = std::make_shared<NormalTexture>(
         m_device->getPhysicalDevice(),
         m_device->getLogicalDevice(),
         info,
         VK_SAMPLE_COUNT_1_BIT,
         m_commandPoolGraphics,
         m_qfHandles.graphicsQueue,
         UsageType::BRDF
   );
}

void Renderer::doComputations()
{
   std::vector<Computation> computations = {
      m_BRDFcomp
   };

   std::cout << "Doing computations.\n";

   const VkCommandBuffer& commandBuffer = (
         m_commandPoolCompute->getCommandBuffer(0)
   );

   for (auto& computation : computations)
   {
      // Resets the command buffer to be able to be recorded.
      m_commandPoolCompute->resetCommandBuffer(0);
      // Specifies some details about the usage of this specific command
      // buffer.
      m_commandPoolCompute->beginCommandBuffer(0, commandBuffer);

         computation.execute(commandBuffer);

         // To make sure that the buffer is ready to be accessed.
         VkMemoryBarrier readBarrier = {
            VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            nullptr,
            // Specifies that we'll wait for the shader to finish writing to the
            // buffer.
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_HOST_READ_BIT
         };

         commandManager::synchronization::recordPipelineBarrier(
               // Specifies that we'll wait for the Compute Queue to finish the
               // execution of the tasks.
               VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
               VK_PIPELINE_STAGE_HOST_BIT,
               0,
               commandBuffer,
               {readBarrier},
               {},
               {}
         );

      m_commandPoolCompute->endCommandBuffer(commandBuffer);

      m_commandPoolCompute->submitCommandBuffer(
            m_qfHandles.computeQueue,
            {commandBuffer},
            true
      );
   }

   std::cout << "All the computations have been completed.\n";

}

void Renderer::destroySyncObjects()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
   {
      vkDestroySemaphore(
            m_device->getLogicalDevice(),
            m_imageAvailableSemaphores[i],
            nullptr
      );
      vkDestroySemaphore(
            m_device->getLogicalDevice(),
            m_renderFinishedSemaphores[i],
            nullptr
      );
      vkDestroyFence(
            m_device->getLogicalDevice(),
            m_inFlightFences[i],
            nullptr
      );
   }
}

void Renderer::cleanup()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // MSAA
   m_msaa.destroy();

   // DepthBuffer
   m_depthBuffer.destroy();

   // ImGui
   m_GUI->destroy();

   // Swapchain
   m_swapchain->destroy();

   // Graphics Pipelines
   m_graphicsPipelineShadowMap.destroy();

   // Computations
   m_BRDFcomp.destroy();

   // Scenes
   m_scene.destroy();

   // Renderpass
   m_renderPassShadowMap.destroy();

   // Models -> Buffers, Memories and Textures.
   m_shadowMap->destroy();


   // Descriptor Pool
   m_descriptorPoolGraphics.destroy();
   m_descriptorPoolComputations.destroy();

   // Sync objects
   destroySyncObjects();

   // Command Pools
   if (m_commandPoolGraphics) m_commandPoolGraphics->destroy();
   if (m_commandPoolCompute)  m_commandPoolCompute->destroy();
   
   // Logical Device
   vkDestroyDevice(m_device->getLogicalDevice(), nullptr);
   
   // Window Surface
   m_window->destroySurface(m_vkInstance->get());
   
   // Vulkan's instance
   m_vkInstance->destroy();

   // GLFW
   m_window->destroy();
}
