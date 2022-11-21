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
#include <VulkanToyRenderer/Settings/vLayersConfig.h>
#include <VulkanToyRenderer/Window/Window.h>
#include <VulkanToyRenderer/ValidationLayers/vlManager.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyHandles.h>
#include <VulkanToyRenderer/Swapchain/Swapchain.h>
#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipeline.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Commands/commandUtils.h>
#include <VulkanToyRenderer/Extensions/extensionsUtils.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Buffers/bufferUtils.h>
#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Model/Types/NormalPBR.h>
#include <VulkanToyRenderer/Model/Types/Skybox.h>
#include <VulkanToyRenderer/Model/Types/Light.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptors/descriptorSetLayoutUtils.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>
#include <VulkanToyRenderer/RenderPass/subPassUtils.h>
#include <VulkanToyRenderer/RenderPass/attachmentUtils.h>
#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/GUI/GUI.h>
#include <VulkanToyRenderer/Camera/Camera.h>
#include <VulkanToyRenderer/Camera/Types/Arcball.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>


void Renderer::run()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif


   loadModels();

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

   if (m_models.size() == 0)
   {
      std::cout << "Nothing to render..\n";
      return;
   }

   m_window = std::make_shared<Window>(
         config::RESOLUTION_W,
         config::RESOLUTION_H,
         config::TITLE
   );

   initVK();

   // Keyword and mouse settings
   m_isMouseInMotion = false;
   
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

   glfwSetWindowUserPointer(m_window->get(), m_camera.get());
   glfwSetScrollCallback(m_window->get(), scrollCallback);


   m_GUI = std::make_unique<GUI>(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         m_vkInstance,
         *(m_swapchain.get()),
         m_qfIndices.graphicsFamily.value(),
         m_qfHandles.graphicsQueue,
         m_window
   );

   mainLoop();

   cleanup();
}

void Renderer::loadModel(const size_t startI, const size_t chunckSize)
{
   const size_t endI = startI + chunckSize;

   for (size_t i = startI; i < endI; i++)
   {
      ModelToLoadInfo& modelInfo = m_modelsToLoadInfo[i];

      switch (modelInfo.type)
      {
         case ModelType::SKYBOX:
         {
            m_models.push_back(
                  std::make_shared<Skybox>(
                     modelInfo.name,
                     modelInfo.modelFileName
                  )
            );
      
            m_skyboxModelIndex.push_back(m_models.size() - 1);
            m_skybox = std::dynamic_pointer_cast<Skybox>(
                  m_models[m_skyboxModelIndex[0]]
            );
      
            break;
      
         }
         case ModelType::NORMAL_PBR:
         {
            m_models.push_back(
                  std::make_shared<NormalPBR>(
                     modelInfo.name,
                     modelInfo.modelFileName,
                     glm::fvec4(modelInfo.pos, 1.0f),
                     modelInfo.rot,
                     modelInfo.size
                  )
            );
            m_objectModelIndices.push_back(m_models.size() - 1);
      
            // TODO: delete this
            m_mainModelIndex = m_models.size() - 1;
      
            break;
      
         }
         case ModelType::LIGHT:
         {
            switch (modelInfo.lType)
            {
               case LightType::DIRECTIONAL_LIGHT:
               {
                  m_models.push_back(
                        std::make_shared<Light>(
                           modelInfo.name,
                           modelInfo.modelFileName,
                           LightType::DIRECTIONAL_LIGHT,
                           glm::fvec4(modelInfo.color, 1.0f),
                           glm::fvec4(modelInfo.pos, 1.0f),
                           glm::fvec4(modelInfo.endPos, 1.0f),
                           glm::fvec3(0.0f),
                           modelInfo.size,
                           0.0f,
                           0.0f
                        )
                  );
                  m_lightModelIndices.push_back(m_models.size() - 1);
                  // TODO: Improve this.
                  m_directionalLightIndex = m_models.size() - 1;
                  
                  break;
      
               }
               case LightType::POINT_LIGHT:
               {
                  m_models.push_back(
                        std::make_shared<Light>(
                           modelInfo.name,
                           modelInfo.modelFileName,
                           LightType::POINT_LIGHT,
                           glm::fvec4(modelInfo.color, 1.0f),
                           glm::fvec4(modelInfo.pos, 1.0f),
                           glm::fvec4(0.0f),
                           glm::fvec3(0.0f),
                           modelInfo.size,
                           modelInfo.attenuation,
                           modelInfo.radius
                        )
                  );
                  m_lightModelIndices.push_back(m_models.size() - 1);
      
                  break;
      
               }
               case LightType::SPOT_LIGHT:
               {
                  m_models.push_back(
                        std::make_shared<Light>(
                           modelInfo.name,
                           modelInfo.modelFileName,
                           LightType::SPOT_LIGHT,
                           glm::fvec4(modelInfo.color, 1.0f),
                           glm::fvec4(modelInfo.pos, 1.0f),
                           glm::fvec4(modelInfo.endPos, 1.0f),
                           modelInfo.rot,
                           modelInfo.size,
                           modelInfo.attenuation,
                           modelInfo.radius
                        )
                  );
                  m_lightModelIndices.push_back(m_models.size() - 1);
      
      
                  break;
      
               }
            }
      
            break;
      
         }
      }
   }
}

void Renderer::loadModels()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif
   
   std::vector<std::thread> threads;

   const size_t maxThreadsCount = std::thread::hardware_concurrency() - 1;
   size_t chunckSize = (
         (m_modelsToLoadInfo.size() < maxThreadsCount) ?
            1 :
            m_modelsToLoadInfo.size() / maxThreadsCount
   );
   const size_t threadsCount = (
         (m_modelsToLoadInfo.size() < maxThreadsCount) ?
            m_modelsToLoadInfo.size() :
            maxThreadsCount
   );

   for (size_t i = 0; i < threadsCount; i++)
   {
      if (i == threadsCount - 1 && maxThreadsCount < m_modelsToLoadInfo.size())
      {
         chunckSize = (
               m_modelsToLoadInfo.size() - (threadsCount * chunckSize)
         );
      }

      threads.push_back(std::thread(&Renderer::loadModel, this, i, chunckSize));
   }

   for (auto& thread : threads)
      thread.join();

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

   if (!m_directionalLightIndex.has_value())
   {
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

   } else
   {
      throw std::runtime_error(
            "Just one directional light per scene is allowed to be added."
      );
   }
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
            m_device.getLogicalDevice(),
            &semaphoreInfo,
            nullptr,
            &m_imageAvailableSemaphores[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create semaphore!");

      status = vkCreateSemaphore(
         m_device.getLogicalDevice(),
         &semaphoreInfo,
         nullptr,
         &m_renderFinishedSemaphores[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create semaphore!");

      status = vkCreateFence(
            m_device.getLogicalDevice(),
            &fenceInfo,
            nullptr,
            &m_inFlightFences[i]
      );

      if (status != VK_SUCCESS)
         throw std::runtime_error("Failed to create fence!");
   }
}

void Renderer::createVkInstance()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED &&
       !vlManager::areAllRequestedLayersAvailable()
   ) {
      throw std::runtime_error(
            "Validation layers requested, but not available!"
      );
   }

   // This data is optional, but it may provide some useful information
   // to the driver in order to optimize our specific application(e.g because
   // it uses a well-known graphics engine with certain special behavior).
   //
   // Example: Our game uses UE and nvidia launched a new driver that optimizes
   // a certain thing. So in that case, nvidia will know it can apply that
   // optimization verifying this info.
   VkApplicationInfo appInfo{};

   appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pApplicationName = config::TITLE;
   appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
   appInfo.pEngineName = "No Engine";
   appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
   appInfo.apiVersion = VK_API_VERSION_1_0;

   // This data is not optional and tells the Vulkan driver which global
   // extensions and validation layers we want to use.
   VkInstanceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   createInfo.pApplicationInfo = &appInfo;

   std::vector<const char*> extensions = (
         extensionsUtils::getRequiredExtensions()
   );

   createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
   createInfo.ppEnabledExtensionNames = extensions.data();

   // This variable is placed outside the if statement to ensure that it is
   // not destroyed before the vkCreateInstance call.
   VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {

      createInfo.enabledLayerCount = static_cast<uint32_t> (
            vLayersConfig::VALIDATION_LAYERS.size()
      );
      createInfo.ppEnabledLayerNames = vLayersConfig::VALIDATION_LAYERS.data();

      debugCreateInfo = vlManager::getDebugMessengerCreateInfo();

      createInfo.pNext = (
            (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo
      );

   } else
   {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = nullptr;
   }

   // -------------------------------------------------------------
   // Paramet. 1 -> Pointer to struct with creation info.
   // Paramet. 2 -> Pointer to custom allocator callbacks.
   // Paramet. 3 -> Pointer to the variable that stores the handle to the
   //               new object.
   // -------------------------------------------------------------
   if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
      throw std::runtime_error("Failed to create Vulkan's instance!");
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
   subpass.inputAttachmentCount = 0;
   subpass.pInputAttachments = nullptr;
   // No color attachments
   subpass.colorAttachmentCount = 0;
   subpass.pColorAttachments = nullptr;
   subpass.pResolveAttachments = nullptr;
   subpass.pDepthStencilAttachment = &shadowMapAttachmentRef;
   subpass.preserveAttachmentCount = 0;
   subpass.pPreserveAttachments = nullptr;

   m_renderPassShadowMap = RenderPass(
         m_device.getLogicalDevice(),
         {shadowMapAttachment},
         {subpass},
         {}
         //dependencies
   );

   
}

void Renderer::createSceneRenderPass()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // -Attachments

   // Color Attachment
   VkAttachmentDescription colorAttachment{};
   attachmentUtils::createAttachmentDescription(
         m_swapchain->getImageFormat(),
         m_msaa.getSamplesCount(),
         VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         colorAttachment
   );

   // Depth Attachment
   VkAttachmentDescription depthAttachment{};
   attachmentUtils::createAttachmentDescriptionWithStencil(
         m_depthBuffer.getFormat(),
         m_msaa.getSamplesCount(),
         VK_ATTACHMENT_LOAD_OP_CLEAR,
         // We don't care about storing the depth data, because it will not be
         // used after drawing has finished.
         VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE,
         // Just like the color buffer, we don't care about the previous depth
         // contents.
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         depthAttachment
   );

   // Color Resolve Attachment(needed by MSAA)
   VkAttachmentDescription colorResolveAttachment{};
   attachmentUtils::createAttachmentDescriptionWithStencil(
         m_swapchain->getImageFormat(),
         VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED,
         // Here is not
         // 'VK_IMAGE_LAYOUT_PRESENT_SRC_KHR'
         // because the GUI will be the last and the one
         // to present.
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         colorResolveAttachment
   );


   // Attachment references
   
   VkAttachmentReference colorAttachmentRef{};
   attachmentUtils::createAttachmentReference(
         0,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         colorAttachmentRef
   );

   VkAttachmentReference depthAttachmentRef{};
   attachmentUtils::createAttachmentReference(
         1,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         depthAttachmentRef
   );

   VkAttachmentReference colorResolveAttachmentRef{};
   attachmentUtils::createAttachmentReference(
         2,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         colorResolveAttachmentRef
   );

   // Subpasses
   VkSubpassDescription subPassDescript{};
   subPassUtils::createSubPassDescription(
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         &colorAttachmentRef,
         &depthAttachmentRef,
         &colorResolveAttachmentRef,
         subPassDescript
   );

   // Subpass dependencies
   VkSubpassDependency dependency{};
   subPassUtils::createSubPassDependency(
         // -Source parameters.
         //VK_SUBPASS_EXTERNAL means anything outside of a given render pass
         //scope. When used for srcSubpass it specifies anything that happened 
         //before the render pass. 
         VK_SUBPASS_EXTERNAL,
         // Operations that the subpass needs to wait on. 
         (
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
         ),
         0,
         // -Destination parameters.
         0,
         (
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
          VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
         ),
         (
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
         ),
         (VkDependencyFlagBits)0,
         dependency
   );

   m_renderPass = RenderPass(
         m_device.getLogicalDevice(),
         {colorAttachment, depthAttachment, colorResolveAttachment},
         {subPassDescript},
         {dependency}
   );
}

/*
 * Uploads the data of each model to the gpu.
 */
void Renderer::uploadModels()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   for (auto& model : m_models)
   {
      // Vertex buffer and index buffer(with staging buffer)
      // (Position, color, texCoord, normal, etc)
      model->uploadVertexData(
            m_device.getPhysicalDevice(),
            m_device.getLogicalDevice(),
            m_qfHandles.graphicsQueue,
            m_commandPool
      );
            
      // Creates and uploads the Textures(images and samplers).
      model->loadTextures(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         VK_SAMPLE_COUNT_1_BIT,
         m_commandPool,
         m_qfHandles.graphicsQueue
      );

      // Uniform Buffers
      model->createUniformBuffers(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         config::MAX_FRAMES_IN_FLIGHT
      );

      // Descriptor Sets
      switch (model->getType())
      {
         case ModelType::SKYBOX:
         {

            if (auto pModel = std::dynamic_pointer_cast<Skybox>(model))
            {
               pModel->createDescriptorSets(
                  m_device.getLogicalDevice(),
                  m_descriptorSetLayoutSkybox,
                  m_descriptorPool
               );
            }

            break;

         } case ModelType::NORMAL_PBR:
         {

            if (auto pModel = std::dynamic_pointer_cast<NormalPBR>(model))
            {
               pModel->createDescriptorSets(
                  m_device.getLogicalDevice(),
                  m_descriptorSetLayoutNormalPBR,
                  m_skybox->getIrradianceMap(),
                  m_shadowMap,
                  m_descriptorPool
               );
            }

            break;

         } case ModelType::LIGHT:
         {
            if (auto pModel = std::dynamic_pointer_cast<Light>(model))
            {
               pModel->createDescriptorSets(
                  m_device.getLogicalDevice(),
                  m_descriptorSetLayoutLight,
                  m_descriptorPool
               );
            }

            break;

         } case ModelType::NONE:
         {

            break;
         }
      }
   }
}

void Renderer::createGraphicsPipelines()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   ///////////////////////////////////MODELS///////////////////////////////////

   m_graphicsPipelineSkybox = GraphicsPipeline(
         m_device.getLogicalDevice(),
         GraphicsPipelineType::SKYBOX,
         m_swapchain->getExtent(),
         m_renderPass.get(),
         m_descriptorSetLayoutSkybox,
         {
            {
               shaderType::VERTEX,
               "skybox"
            },
            {
               shaderType::FRAGMENT,
               "skybox"
            }
         },
         m_msaa.getSamplesCount(),
         Attributes::SKYBOX::getBindingDescription(),
         Attributes::SKYBOX::getAttributeDescriptions(),
         &m_skyboxModelIndex
   );

   m_graphicsPipelinePBR = GraphicsPipeline(
         m_device.getLogicalDevice(),
         GraphicsPipelineType::PBR,
         m_swapchain->getExtent(),
         m_renderPass.get(),
         m_descriptorSetLayoutNormalPBR,
         {
            {
               shaderType::VERTEX,
               // Filename of the vertex shader.
               "scene"
            },
            {
               shaderType::FRAGMENT,
               // Filename of the fragment shader.
               "scene"
            }
         },
         m_msaa.getSamplesCount(),
         Attributes::PBR::getBindingDescription(),
         Attributes::PBR::getAttributeDescriptions(),
         // Models assocciated with this graphics pipeline.
         &m_objectModelIndices
   );


   m_graphicsPipelineLight = GraphicsPipeline(
         m_device.getLogicalDevice(),
         GraphicsPipelineType::LIGHT,
         m_swapchain->getExtent(),
         m_renderPass.get(),
         m_descriptorSetLayoutLight,
         {
            {
               shaderType::VERTEX,
               "light"
            },
            {
               shaderType::FRAGMENT,
               "light"
            }
         },
         m_msaa.getSamplesCount(),
         Attributes::LIGHT::getBindingDescription(),
         Attributes::LIGHT::getAttributeDescriptions(),
         // Models assocciated with this graphics pipeline.
         &m_lightModelIndices
   );

   //////////////////////////////////FEATURES//////////////////////////////////

   m_graphicsPipelineShadowMap = GraphicsPipeline(
         m_device.getLogicalDevice(),
         GraphicsPipelineType::SHADOWMAP,
         m_swapchain->getExtent(),
         m_renderPassShadowMap.get(),
         m_descriptorSetLayoutShadowMap,
         {
            {
               shaderType::VERTEX,
               "shadowMap"
            }
         },
         VK_SAMPLE_COUNT_1_BIT,
         Attributes::PBR::getBindingDescription(),
         Attributes::PBR::getAttributeDescriptions(),
         &m_objectModelIndices
   );
}

/*
 * Specifies all the neccessary descriptors, their bindings and their 
 * shader stages.
 */
void Renderer::createDescriptorSetLayouts()
{
   
#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // ---------------------------------Models----------------------------------

   descriptorSetLayoutUtils::createDescriptorSetLayout(
         m_device.getLogicalDevice(),
         GRAPHICS_PIPELINE::SKYBOX::UBOS_INFO,
         GRAPHICS_PIPELINE::SKYBOX::SAMPLERS_INFO,
         m_descriptorSetLayoutSkybox
   );


   descriptorSetLayoutUtils::createDescriptorSetLayout(
         m_device.getLogicalDevice(),
         GRAPHICS_PIPELINE::PBR::UBOS_INFO,
         GRAPHICS_PIPELINE::PBR::SAMPLERS_INFO,
         m_descriptorSetLayoutNormalPBR
   );
   
   descriptorSetLayoutUtils::createDescriptorSetLayout(
         m_device.getLogicalDevice(),
         GRAPHICS_PIPELINE::LIGHT::UBOS_INFO,
         GRAPHICS_PIPELINE::LIGHT::SAMPLERS_INFO,
         m_descriptorSetLayoutLight
   );


   // ---------------------------------Features--------------------------------

   descriptorSetLayoutUtils::createDescriptorSetLayout(
         m_device.getLogicalDevice(),
         GRAPHICS_PIPELINE::SHADOWMAP::UBOS_INFO,
         {},
         m_descriptorSetLayoutShadowMap
   );
}

void Renderer::initVK()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   createVkInstance();

   vlManager::setupDebugMessenger(m_vkInstance, m_debugMessenger);

   m_window->createSurface(m_vkInstance);

   m_device.pickPhysicalDevice(
         m_vkInstance,
         m_qfIndices,
         m_window->getSurface()
   );

   m_device.createLogicalDevice(m_qfIndices);
   
   m_qfHandles.setQueueHandles(m_device.getLogicalDevice(), m_qfIndices);

   m_swapchain = std::make_unique<Swapchain>(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         m_window,
         m_device.getSupportedProperties()
   );

   m_swapchain->createAllImageViews();
   
   // Descriptor Pool
   // (Calculates the total size of the pool depending of the descriptors
   // we send as parameter and the number of descriptor SETS defined)
   m_descriptorPool = DescriptorPool(
         m_device.getLogicalDevice(),
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

   createDescriptorSetLayouts();
   
   // -----------------------------------Features------------------------------

   m_msaa = MSAA(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         m_swapchain->getExtent(),
         m_swapchain->getImageFormat()
   );

   m_depthBuffer = DepthBuffer(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         m_swapchain->getExtent(),
         m_msaa.getSamplesCount()
   );

   m_shadowMap = std::make_shared<ShadowMap>(
         m_device.getPhysicalDevice(),
         m_device.getLogicalDevice(),
         m_swapchain->getExtent().width,
         m_swapchain->getExtent().height,
         m_depthBuffer.getFormat(),
         m_descriptorSetLayoutShadowMap,
         config::MAX_FRAMES_IN_FLIGHT
   );

   //----------------------------------RenderPass------------------------------

   createShadowMapRenderPass();
   createSceneRenderPass();

   //----------------------------------Framebuffers----------------------------

   m_swapchain->createFramebuffers(
         m_renderPass.get(),
         m_depthBuffer,
         m_msaa
   );
   m_shadowMap->createFramebuffer(
         m_renderPassShadowMap.get(),
         m_swapchain->getImageCount()
   );

   //--------------------------------------------------------------------------

   createGraphicsPipelines();

   //----------------------------------Command Pools---------------------------
   m_commandPool = std::make_shared<CommandPool>(
         m_device.getLogicalDevice(),
         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         m_qfIndices.graphicsFamily.value()
   );
   
   // Allocates all the neccessary cmd buffers in the cmd pool.
   m_commandPool->allocCommandBuffers(
         config::MAX_FRAMES_IN_FLIGHT
   );

   m_shadowMap->createCommandPool(
         VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         m_qfIndices.graphicsFamily.value()
   );

   m_shadowMap->allocCommandBuffers(
         config::MAX_FRAMES_IN_FLIGHT
   );

   //--------------------------------------------------------------------------
   
   uploadModels();
     
   createSyncObjects();

}

template <typename T>
void Renderer::bindAllMeshesData(
      const std::shared_ptr<T>& model,
      const GraphicsPipeline& graphicsPipeline,
      const VkCommandBuffer& commandBuffer,
      const uint32_t currentFrame
) {

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   for (auto& mesh : model->m_meshes)
   {
      commandUtils::STATE::bindVertexBuffers(
            {mesh.vertexBuffer},
            // Offsets.
            {0},
            // Index of first binding.
            0,
            // Bindings count.
            1,
            commandBuffer
      );
      commandUtils::STATE::bindIndexBuffer(
            mesh.indexBuffer,
            // Offset.
            0,
            VK_INDEX_TYPE_UINT32,
            commandBuffer
      );

      // TODO: Improve this.
      VkDescriptorSet descriptorSet;
      if (graphicsPipeline.getType() == GraphicsPipelineType::SHADOWMAP)
         descriptorSet = m_shadowMap->getDescriptorSet(currentFrame);
      else
         descriptorSet = mesh.descriptorSets.get(currentFrame);

      commandUtils::STATE::bindDescriptorSets(
            graphicsPipeline.getPipelineLayout(),
            // Index of first descriptor set.
            0,
            {descriptorSet},
            // Dynamic offsets.
            {},
            commandBuffer
      );

      commandUtils::ACTION::drawIndexed(
            // Index Count
            mesh.indices.size(),
            // Instance Count
            1,
            // First index.
            0,
            // Vertex Offset.
            0,
            // First Intance.
            0,
            commandBuffer
      );
   }
}

void Renderer::recordCommandBuffer(
      const VkFramebuffer& framebuffer,
      const RenderPass& renderPass,
      const VkExtent2D& extent,
      const std::vector<GraphicsPipeline>& graphicsPipelines,
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

      // The final parameter controls how the drawing commands between the
      // render pass will be provided:
      //    -VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be
      //    embedded in the primary command buffer itself and no secondary
      //    command buffers will be executed.
      //    -VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass
      //    commands will be executed from secondary command buffers.
      renderPass.begin(
            framebuffer,
            extent,
            clearValues,
            commandBuffer,
            VK_SUBPASS_CONTENTS_INLINE
      );

         //---------------------------------CMDs-------------------------------

         for (const auto& graphicsPipeline : graphicsPipelines)
         {
            commandUtils::STATE::bindPipeline(
                  graphicsPipeline.get(),
                  commandBuffer
            );
            // Set Dynamic States
            commandUtils::STATE::setViewport(
                  0.0f,
                  0.0f,
                  extent,
                  0.0f,
                  1.0f,
                  0,
                  1,
                  commandBuffer
            );
            commandUtils::STATE::setScissor(
                  {0, 0},
                  extent,
                  0,
                  1,
                  commandBuffer
            );

            // Binds all the models with the same Graphics Pipeline.
            for (const size_t& i : graphicsPipeline.getModelIndices())
            {
               const auto& model = m_models[i];

               switch (model->getType())
               {
                  case ModelType::SKYBOX:
                  {
                     if (auto pModel = std::dynamic_pointer_cast<Skybox>(model))
                        bindAllMeshesData<Skybox>(
                              pModel,
                              graphicsPipeline,
                              commandBuffer,
                              currentFrame
                        );

                     break;

                  } case ModelType::NORMAL_PBR:
                  {
                     if (auto pModel = std::dynamic_pointer_cast<NormalPBR>(model))
                     {
                        if (pModel.get()->isHided())
                           continue;

                        bindAllMeshesData<NormalPBR>(
                              pModel,
                              graphicsPipeline,
                              commandBuffer,
                              currentFrame
                        );
                     }

                     break;

                  } case ModelType::LIGHT:
                  {
                     if (auto pModel =std::dynamic_pointer_cast<Light>(model))
                     {
                        if (pModel.get()->isHided())
                           continue;
                     
                        bindAllMeshesData<Light>(
                           pModel, graphicsPipeline, commandBuffer, currentFrame
                        );
                     }

                     break;

                  } case ModelType::NONE:
                  {
                     break;
                  }
               }
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
         m_device.getLogicalDevice(),
         1,
         &m_inFlightFences[currentFrame],
         VK_TRUE,
         UINT64_MAX
   );

   // After waiting, we need to manually reset the fence.
   vkResetFences(
         m_device.getLogicalDevice(),
         1,
         &m_inFlightFences[currentFrame]
   );

   //------------------------Updates uniform buffer----------------------------

   // Shadow Map
   if (m_directionalLightIndex.has_value())
   {
      auto pLight = std::dynamic_pointer_cast<Light>(
            m_models[m_directionalLightIndex.value()]
      );
      // TODO: improve this
      auto pMainModel = std::dynamic_pointer_cast<NormalPBR>(
            m_models[m_mainModelIndex]
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

   // Scene
   for (auto& model : m_models)
   {

      switch (model->getType())
      {
         case ModelType::SKYBOX:
         {
            if (auto pModel = std::dynamic_pointer_cast<Skybox>(model))
               pModel->updateUBO(
                     m_device.getLogicalDevice(),
                     m_camera->getPos(),
                     m_camera->getViewM(),
                     m_swapchain->getExtent(),
                     currentFrame
               );

            break;

         } case ModelType::NORMAL_PBR:
         {
            if (auto pModel = std::dynamic_pointer_cast<NormalPBR>(model))
            {
               pModel->updateUBO(
                     m_device.getLogicalDevice(),
                     m_camera->getPos(),
                     m_camera->getViewM(),
                     m_camera->getProjectionM(),
                     m_shadowMap->getLightSpace(),
                     m_lightModelIndices.size(),
                     m_models,
                     currentFrame
               );
               pModel->updateUBOlightsInfo(
                     m_device.getLogicalDevice(),
                     m_lightModelIndices,
                     m_models,
                     currentFrame
               );
            }
         } case ModelType::LIGHT:
         {
            if (auto pModel = std::dynamic_pointer_cast<Light>(model))
            {
               pModel->updateUBO(
                     m_device.getLogicalDevice(),
                     m_camera->getPos(),
                     m_camera->getViewM(),
                     m_camera->getProjectionM(),
                     currentFrame
               );
            }
 
            break;

         } case ModelType::NONE:
         {
            break;
         }
      }
   }

   //--------------------Acquires an image from the swapchain------------------
   uint32_t imageIndex;
   vkAcquireNextImageKHR(
         m_device.getLogicalDevice(),
         m_swapchain->get(),
         UINT64_MAX,
         // Specifies synchr. objects that have to be signaled when the
         // presentation engine is finished using the image.
         m_imageAvailableSemaphores[currentFrame],
         VK_NULL_HANDLE,
         &imageIndex
   );

   //---------------------Records all the command buffer-----------------------

   // Shadow Mapping
   recordCommandBuffer(
         m_shadowMap->getFramebuffer(imageIndex),
         m_renderPassShadowMap,
         m_swapchain->getExtent(),
         {
            m_graphicsPipelineShadowMap
         },
         currentFrame,
         m_shadowMap->getCommandBuffer(currentFrame),
         m_clearValuesShadowMap,
         m_shadowMap->getCommandPool()
   );
   // Scene
   recordCommandBuffer(
         m_swapchain->getFramebuffer(imageIndex),
         m_renderPass,
         m_swapchain->getExtent(),
         {
            m_graphicsPipelineLight,
            m_graphicsPipelinePBR,
            // The skybox has to be always the last one.
            m_graphicsPipelineSkybox
         },
         currentFrame,
         m_commandPool->getCommandBuffer(currentFrame),
         m_clearValues,
         m_commandPool
   );

   // GUI
   m_GUI->recordCommandBuffer(currentFrame,  imageIndex, m_clearValues);

   //----------------------Submits the command buffer--------------------------

   VkSubmitInfo submitInfo{};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   VkSemaphore waitSemaphores[] = {
      m_imageAvailableSemaphores[currentFrame]
   };
   VkPipelineStageFlags waitStages [] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
   };
   submitInfo.waitSemaphoreCount = 1;
   // Specifies which semaphores to wait on before execution begins.
   submitInfo.pWaitSemaphores = waitSemaphores;
   // Specifies which stage/s of the pipeline to wait.
   submitInfo.pWaitDstStageMask = waitStages;
   // These two commands specify which command buffers to actualy submit for
   // execution.
   
   std::vector<VkCommandBuffer> submitCommandBuffers = {
      m_shadowMap->getCommandBuffer(currentFrame),
      m_commandPool->getCommandBuffer(currentFrame),
      m_GUI->getCommandBuffer(currentFrame)
   };

   submitInfo.commandBufferCount = submitCommandBuffers.size();
   submitInfo.pCommandBuffers = submitCommandBuffers.data();
   // Specifies which semaphores to signal once the command buffer/s have
   // finished execution.
   VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[currentFrame]};
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = signalSemaphores;

   auto status = vkQueueSubmit(
         m_qfHandles.graphicsQueue,
         1,
         &submitInfo,
         m_inFlightFences[currentFrame]
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to submit draw command buffer!");

   //-------------------Presentation of the swapchain image--------------------
   
   VkPresentInfoKHR presentInfo{};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   // Specifies which semaphores to wait on before the presentation can happen.
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = signalSemaphores;

   // Specifies the swapchains to present images to and the index of the
   // image for each swapchain.
   VkSwapchainKHR swapchains[] = {m_swapchain->get()};
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = swapchains;
   presentInfo.pImageIndices = &imageIndex;
   // Allows us to specify an array of VkResult values to check for every
   // individual swapchain if presentation was successful.
   // Optional
   presentInfo.pResults = nullptr;

   vkQueuePresentKHR(m_qfHandles.presentQueue, &presentInfo);

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

   // TODO
   if (newFOV || newFOV)
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

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   // Tells us in which frame we are,
   // between 1 <= frame <= MAX_FRAMES_IN_FLIGHT
   uint8_t currentFrame = 0;
   while (m_window->isWindowClosed() == false)
   {
      handleInput();

      // Draws Imgui
      m_GUI->draw(
            m_models,
            m_camera,
            m_objectModelIndices,
            m_lightModelIndices
      );
      drawFrame(currentFrame);
   }
   vkDeviceWaitIdle(m_device.getLogicalDevice());
}

void Renderer::destroySyncObjects()
{

#ifdef RELEASE_MODE_ON
   ZoneScoped;
#endif

   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
   {
      vkDestroySemaphore(
            m_device.getLogicalDevice(),
            m_imageAvailableSemaphores[i],
            nullptr
      );
      vkDestroySemaphore(
            m_device.getLogicalDevice(),
            m_renderFinishedSemaphores[i],
            nullptr
      );
      vkDestroyFence(
            m_device.getLogicalDevice(),
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
   m_graphicsPipelinePBR.destroy();
   m_graphicsPipelineSkybox.destroy();
   m_graphicsPipelineLight.destroy();
   m_graphicsPipelineShadowMap.destroy();

   // Renderpass
   m_renderPass.destroy();
   m_renderPassShadowMap.destroy();

   // Models -> Buffers, Memories and Textures.
   m_shadowMap->destroy();
   for (auto& model : m_models)
      model->destroy(m_device.getLogicalDevice());

   // Descriptor Pool
   m_descriptorPool.destroy();

   // Descriptor Set Layouts
   descriptorSetLayoutUtils::destroyDescriptorSetLayout(
         m_device.getLogicalDevice(),
         m_descriptorSetLayoutNormalPBR
   );
   descriptorSetLayoutUtils::destroyDescriptorSetLayout(
         m_device.getLogicalDevice(),
         m_descriptorSetLayoutSkybox
   );
   descriptorSetLayoutUtils::destroyDescriptorSetLayout(
         m_device.getLogicalDevice(),
         m_descriptorSetLayoutLight
   );
   descriptorSetLayoutUtils::destroyDescriptorSetLayout(
         m_device.getLogicalDevice(),
         m_descriptorSetLayoutShadowMap
   );
   
   // Sync objects
   destroySyncObjects();

   // Command Pool
   m_commandPool->destroy();
   
   // Logical Device
   vkDestroyDevice(m_device.getLogicalDevice(), nullptr);
   
   // Validation Layers
   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {
         vlManager::destroyDebugUtilsMessengerEXT(
               m_vkInstance,
               m_debugMessenger,
               nullptr
         );
   }

   // Window Surface
   m_window->destroySurface(m_vkInstance);
   
   // Vulkan's instance
   vkDestroyInstance(m_vkInstance, nullptr);

   // GLFW
   m_window->destroy();
}
