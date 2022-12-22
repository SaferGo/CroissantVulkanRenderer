#include <CroissantRenderer/Scene/Scene.h>

#include <thread>
#include <iostream>

#include <CroissantRenderer/Texture/Type/NormalTexture.h>

Scene::Scene() {}

Scene::Scene(
      const VkDevice& logicalDevice,
      const VkFormat& format,
      const VkExtent2D& extent,
      const VkSampleCountFlagBits& msaaSamplesCount,
      const VkFormat& depthBufferFormat,
      const std::vector<ModelInfo>& modelsToLoadInfo,
      // Parameters needed for the computations.
      const VkPhysicalDevice& physicalDevice,
      const QueueFamilyIndices& queueFamilyIndices,
      DescriptorPool& descriptorPoolForComputations
) : m_logicalDevice(logicalDevice),
    m_mainModelIndex(-1),
    m_directionalLightIndex(-1)
{
   loadModels(modelsToLoadInfo);

   createRenderPass(format, msaaSamplesCount, depthBufferFormat);

   createPipelines(format, extent, msaaSamplesCount);

   initComputations(
         physicalDevice,
         queueFamilyIndices,
         descriptorPoolForComputations
   );
}

void Scene::initComputations(
      const VkPhysicalDevice& physicalDevice,
      const QueueFamilyIndices& queueFamilyIndices,
      DescriptorPool& descriptorPoolForComputations
) {
   m_BRDFcomp = Computation(
         physicalDevice,
         m_logicalDevice,
         "BRDF",
         sizeof(float),
         2 * sizeof(float) * config::BRDF_HEIGHT * config::BRDF_WIDTH,
         queueFamilyIndices,
         descriptorPoolForComputations,
         COMPUTE_PIPELINE::BRDF::BUFFERS_INFO
   );
}

Scene::~Scene() {}

void Scene::createRenderPass(
      const VkFormat& format,
      const VkSampleCountFlagBits& msaaSamplesCount,
      const VkFormat& depthBufferFormat
) {
   // -Attachments

   // Color Attachment
   VkAttachmentDescription colorAttachment{};
   attachmentUtils::createAttachmentDescription(
         format,
         msaaSamplesCount,
         VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         colorAttachment
   );

   // Depth Attachment
   VkAttachmentDescription depthAttachment{};
   attachmentUtils::createAttachmentDescriptionWithStencil(
         depthBufferFormat,
         msaaSamplesCount,
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
         format,
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
         m_logicalDevice,
         {colorAttachment, depthAttachment, colorResolveAttachment},
         {subPassDescript},
         {dependency}
   );
}

void Scene::createPipelines(
      const VkFormat& format,
      const VkExtent2D& extent,
      const VkSampleCountFlagBits& msaaSamplesCount
) {

   m_graphicsPipelineSkybox = Graphics(
         m_logicalDevice,
         GraphicsPipelineType::SKYBOX,
         extent,
         m_renderPass,
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
         msaaSamplesCount,
         Attributes::SKYBOX::getBindingDescription(),
         Attributes::SKYBOX::getAttributeDescriptions(),
         m_skyboxModelIndex,
         GRAPHICS_PIPELINE::SKYBOX::UBOS_INFO,
         GRAPHICS_PIPELINE::SKYBOX::SAMPLERS_INFO,
         {}
   );
   
   m_graphicsPipelinePBR = Graphics(
         m_logicalDevice,
         GraphicsPipelineType::PBR,
         extent,
         m_renderPass,
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
         msaaSamplesCount,
         Attributes::PBR::getBindingDescription(),
         Attributes::PBR::getAttributeDescriptions(),
         // Models assocciated with this graphics pipeline.
         m_objectModelIndices,
         GRAPHICS_PIPELINE::PBR::UBOS_INFO,
         GRAPHICS_PIPELINE::PBR::SAMPLERS_INFO,
         {}
   );

   m_graphicsPipelineLight = Graphics(
         m_logicalDevice,
         GraphicsPipelineType::LIGHT,
         extent,
         m_renderPass,
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
         msaaSamplesCount,
         Attributes::LIGHT::getBindingDescription(),
         Attributes::LIGHT::getAttributeDescriptions(),
         // Models assocciated with this graphics pipeline.
         m_lightModelIndices,
         GRAPHICS_PIPELINE::LIGHT::UBOS_INFO,
         GRAPHICS_PIPELINE::LIGHT::SAMPLERS_INFO,
         {}
   );
}

void Scene::loadModels(const std::vector<ModelInfo>& modelsToLoadInfo)
{
   std::vector<std::thread> threads;

   const size_t maxThreadsCount = std::thread::hardware_concurrency() - 1;
   size_t chunckSize = (
         (modelsToLoadInfo.size() < maxThreadsCount) ?
            1 :
            modelsToLoadInfo.size() / maxThreadsCount
   );
   const size_t threadsCount = (
         (modelsToLoadInfo.size() < maxThreadsCount) ?
            modelsToLoadInfo.size() :
            maxThreadsCount
   );

   for (size_t i = 0; i < threadsCount; i++)
   {
      if (i == threadsCount - 1 && maxThreadsCount < modelsToLoadInfo.size())
      {
         chunckSize += (
               modelsToLoadInfo.size() - (threadsCount * chunckSize)
         );
      }

      threads.push_back(
            std::thread(
               &Scene::loadModel,
               this,
               i,
               chunckSize,
               modelsToLoadInfo
            )
      );
   }

   for (auto& thread : threads)
      thread.join();

   if (m_objectModelIndices.size() == 0)
      throw std::runtime_error(
            "Add at least 1 model."
      );
   if (m_directionalLightIndex == -1)
      throw std::runtime_error(
            "Add at least 1 directional light."
      );
   if (m_skyboxModelIndex.size() == 0)
      throw std::runtime_error(
            "Add at least 1 skybox."
      );
   if (m_skyboxModelIndex.size() > 1)
      throw std::runtime_error(
            "You can't add more than 1 skybox per scene."
      );
}

void Scene::loadModel(
      const size_t startI,
      const size_t chunckSize,
      const std::vector<ModelInfo>& modelsToLoadInfo
) {
   const size_t endI = startI + chunckSize;

   for (size_t i = startI; i < endI; i++)
   {
      const ModelInfo& modelInfo = modelsToLoadInfo[i];

      switch (modelInfo.type)
      {
         case ModelType::SKYBOX:
         {
            m_models.push_back(std::make_shared<Skybox>(modelInfo));
      
            m_skyboxModelIndex.push_back(m_models.size() - 1);
            m_skybox = std::dynamic_pointer_cast<Skybox>(
                  m_models[m_skyboxModelIndex[0]]
            );
      
            break;
      
         } case ModelType::NORMAL_PBR:
         {
            m_models.push_back(std::make_shared<NormalPBR>(modelInfo));
            m_objectModelIndices.push_back(m_models.size() - 1);
      
            // Just the first model added will be shadowable.
            if (m_mainModelIndex == -1)
               m_mainModelIndex = m_models.size() - 1;
      
            break;
      
         } case ModelType::LIGHT:
         {
            m_models.push_back(std::make_shared<Light>(modelInfo));
            m_lightModelIndices.push_back(m_models.size() - 1);

            if (modelInfo.lType == LightType::DIRECTIONAL_LIGHT)
            {
               if (m_directionalLightIndex != -1)
                  throw std::runtime_error(
                        "You can't add more than 1 directional light per scene!"
                  );
               
               m_directionalLightIndex = m_models.size() - 1;
            }

            break;
         }
      }
   }
}

const std::shared_ptr<Model>& Scene::getDirectionalLight() const
{
   return m_models[m_directionalLightIndex];
}

const std::shared_ptr<Model>& Scene::getMainModel() const
{
   return m_models[m_mainModelIndex];
}

const Graphics& Scene::getPBRpipeline() const
{
   return m_graphicsPipelinePBR;
}

const Graphics& Scene::getSkyboxPipeline() const
{
   return m_graphicsPipelineSkybox;
}

const Graphics& Scene::getLightPipeline() const
{
   return m_graphicsPipelineLight;
}

void Scene::updateUBO(
      const std::shared_ptr<Camera>& camera,
      // From the shadow map
      const glm::mat4& lightSpace,
      const VkExtent2D& extent,
      const uint32_t& currentFrame
) {

   UBOinfo uboInfo = {
      camera->getPos(),
      camera->getViewM(),
      camera->getProjectionM(),
      lightSpace,
      static_cast<uint32_t>(m_lightModelIndices.size()),
      extent
   };

   // Scene
   for (auto& model : m_models)
   {
      model->updateUBO(
            m_logicalDevice,
            currentFrame,
            uboInfo
      );

      if (auto pModel = std::dynamic_pointer_cast<NormalPBR>(model))
      {
         pModel->updateUBOlights(
               m_logicalDevice,
               m_lightModelIndices,
               m_models,
               currentFrame
         );
      }
   }

}

const std::vector<std::shared_ptr<Model>>& Scene::getModels() const
{
   return m_models;
}

const std::shared_ptr<Model>& Scene::getModel(uint32_t i) const
{
   return m_models[i];
}

const RenderPass& Scene::getRenderPass() const
{
   return m_renderPass;
}

void Scene::upload(
      const VkPhysicalDevice& physicalDevice,
      const VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool,
      DescriptorPool& descriptorPool,
      // Features
      const std::shared_ptr<ShadowMap<Attributes::PBR::Vertex>> shadowMap
) {
   // First we upload the skybox because we need some dependencies from it for
   // the descriptor sets of the other models.
   m_skybox->upload(
         physicalDevice,
         m_logicalDevice,
         graphicsQueue,
         commandPool,
         config::MAX_FRAMES_IN_FLIGHT
   );

   m_skybox->createDescriptorSets(
         m_logicalDevice,
         m_graphicsPipelineSkybox.getDescriptorSetLayout(),
         nullptr,
         descriptorPool
   );

   // IBL
   {
      loadBRDFlut(physicalDevice, graphicsQueue, commandPool);


      m_prefilteredEnvMap = std::make_shared<
         PrefilteredEnvMap<Attributes::SKYBOX::Vertex>
      >(
            physicalDevice,
            m_logicalDevice,
            graphicsQueue,
            commandPool,
            config::PREF_ENV_MAP_DIM,
            m_skybox->getMeshes(),
            m_skybox->getEnvMap()
      );
   }

   // TODO: Improve this.
   VkDescriptorSetLayout descriptorSetLayout;
   DescriptorSetInfo descriptorSetInfo = {
      &(*m_skybox->getIrradianceMap()),
      &(*m_BRDFlut),
      &(shadowMap->getShadowMapView()),
      &(shadowMap->getSampler()),
      &(m_prefilteredEnvMap->get())
   };

   for (auto& model : m_models)
   {
      auto type = model->getType();

      if (type == ModelType::SKYBOX)
         continue;

      model->upload(
            physicalDevice,
            m_logicalDevice,
            graphicsQueue,
            commandPool,
            // UBO count
            config::MAX_FRAMES_IN_FLIGHT
      );

      // Descriptor Sets
      if (type == ModelType::NORMAL_PBR)
      {
         descriptorSetLayout = (
               m_graphicsPipelinePBR.getDescriptorSetLayout()
         );
  
      } else
      {
         if (type == ModelType::LIGHT)
            descriptorSetLayout = (
                  m_graphicsPipelineLight.getDescriptorSetLayout()
            );
      }

      model->createDescriptorSets(
         m_logicalDevice,
         descriptorSetLayout,
         &descriptorSetInfo,
         descriptorPool
      );
   }
}

void Scene::destroy()
{
   for (auto& model : m_models)
      model->destroy(m_logicalDevice);

   m_graphicsPipelinePBR.destroy();
   m_graphicsPipelineSkybox.destroy();
   m_graphicsPipelineLight.destroy();

   m_renderPass.destroy();

   // IBL
   m_BRDFcomp.destroy();
   m_BRDFlut->destroy();
   m_prefilteredEnvMap->destroy();
}

void Scene::loadBRDFlut(
      const VkPhysicalDevice& physicalDevice,
      const VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool
) {
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
         m_skybox->getTextureFolderName() +
         "/" + 
         "BRDFlut.ktx"
   );

   gli::save_ktx(
         lutTexture,
         pathToTexture
   );

   TextureToLoadInfo info = {
            "BRDFlut.ktx",
            m_skybox->getTextureFolderName(),
            VK_FORMAT_R16G16_SFLOAT,
            4
   };

   m_BRDFlut = std::make_shared<NormalTexture>(
         physicalDevice,
         m_logicalDevice,
         info,
         VK_SAMPLE_COUNT_1_BIT,
         commandPool,
         graphicsQueue,
         UsageType::BRDF
   );
}

// In the future, it'll return a vector of computations.
const Computation& Scene::getComputation() const
{
   return m_BRDFcomp;
}

const std::vector<size_t>& Scene::getObjectModelIndices() const
{
   return m_objectModelIndices;
}

const std::vector<size_t>& Scene::getLightModelIndices() const
{
   return m_lightModelIndices;
}
