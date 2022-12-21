#pragma once

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Model/Model.h>
#include <CroissantRenderer/Model/Types/Skybox.h>
#include <CroissantRenderer/Model/Types/NormalPBR.h>
#include <CroissantRenderer/Model/Types/Light.h>
#include <CroissantRenderer/Model/ModelInfo.h>
#include <CroissantRenderer/Camera/Camera.h>
#include <CroissantRenderer/RenderPass/RenderPass.h>
#include <CroissantRenderer/RenderPass/attachmentUtils.h>
#include <CroissantRenderer/RenderPass/subPassUtils.h>
#include <CroissantRenderer/Settings/graphicsPipelineConfig.h>
#include <CroissantRenderer/Settings/computePipelineConfig.h>
#include <CroissantRenderer/Computation/Computation.h>
#include <CroissantRenderer/Features/PrefilteredEnvMap.h>

class Scene
{
public:

   Scene();
   Scene(
         const VkDevice& logicalDevice,
         const VkFormat& format,
         const VkExtent2D& extent,
         const VkSampleCountFlagBits& msaaSamplesCount,
         const VkFormat& depthBufferFormat,
         const std::vector<ModelInfo>& modelsToLoadInfo,
         // Parameters needed by the computations.
         const VkPhysicalDevice& physicalDevice,
         const QueueFamilyIndices& queueFamilyIndices,
         DescriptorPool& descriptorPoolForComputations
   );
   ~Scene();
   void upload(
         const VkPhysicalDevice& physicalDevice,
         const VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool,
         DescriptorPool& descriptorPool,
         // Features
         const std::shared_ptr<ShadowMap<Attributes::PBR::Vertex>> shadowMap
   );
   void updateUBO(
         const std::shared_ptr<Camera>& camera,
         // From the shadow map
         const glm::mat4& lightSpace,
         const VkExtent2D& extent,
         const uint32_t& currentFrame
   );
   const RenderPass& getRenderPass() const;
   const std::shared_ptr<Model>& getDirectionalLight() const;
   const std::shared_ptr<Model>& getMainModel() const;
   const Graphics& getPBRpipeline() const;
   const Graphics& getSkyboxPipeline() const;
   const Graphics& getLightPipeline() const;
   const std::vector<std::shared_ptr<Model>>& getModels() const;
   const std::shared_ptr<Model>& getModel(uint32_t i) const;
   const std::vector<size_t>& getObjectModelIndices() const;
   const std::vector<size_t>& getLightModelIndices() const;
   const Computation& getComputation() const;
   void destroy();

private:

   void loadModels(const std::vector<ModelInfo>& modelsToLoadInfo);
   void loadModel(
         const size_t startI,
         const size_t chunckSize,
         const std::vector<ModelInfo>& modelsToLoadInfo
   );
   void initComputations(
         const VkPhysicalDevice& physicalDevice,
         const QueueFamilyIndices& queueFamilyIndices,
         DescriptorPool& descriptorPoolForComputations
   );
   void loadBRDFlut(
         const VkPhysicalDevice& physicalDevice,
         const VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool
   );
   void createPipelines(
         const VkFormat& format,
         const VkExtent2D& extent,
         const VkSampleCountFlagBits& msaaSamplesCount
   );
   void createRenderPass(
         const VkFormat& format,
         const VkSampleCountFlagBits& msaaSamplesCount,
         const VkFormat& depthBufferFormat
   );

   VkDevice                            m_logicalDevice;
   RenderPass                          m_renderPass;
   Graphics                            m_graphicsPipelinePBR;
   Graphics                            m_graphicsPipelineSkybox;
   Graphics                            m_graphicsPipelineLight;

   std::vector<std::shared_ptr<Model>> m_models;

   std::shared_ptr<Skybox>             m_skybox;
   std::vector<size_t>                 m_objectModelIndices;
   std::vector<size_t>                 m_lightModelIndices;
   std::vector<size_t>                 m_skyboxModelIndex;

   // For now, this will be the only shadowable model of the scene.
   int                                 m_mainModelIndex;
   int                                 m_directionalLightIndex;


   // IBL
   Computation                         m_BRDFcomp;
   std::shared_ptr<Texture>            m_BRDFlut;
   std::shared_ptr<
      PrefilteredEnvMap<Attributes::SKYBOX::Vertex>
   > m_prefilteredEnvMap;
};
