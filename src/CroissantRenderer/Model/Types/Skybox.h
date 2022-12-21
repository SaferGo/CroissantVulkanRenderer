#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <CroissantRenderer/Model/Model.h>
#include <CroissantRenderer/Model/Attributes.h>
#include <CroissantRenderer/Model/ModelInfo.h>
#include <CroissantRenderer/Texture/Type/Cubemap.h>
#include <CroissantRenderer/Descriptor/DescriptorPool.h>
#include <CroissantRenderer/Descriptor/DescriptorSets.h>
#include <CroissantRenderer/Descriptor/Types/UBO/UBO.h>
#include <CroissantRenderer/Features/ShadowMap.h>

class Skybox : public Model
{

public:

   Skybox(const ModelInfo& modelInfo);
   ~Skybox() override;
   void destroy(const VkDevice& logicalDevice);
   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorSetInfo* info,
         DescriptorPool& descriptorPool
   ) override;
   void bindData(
         const Graphics* graphicsPipeline,
         const VkCommandBuffer& commandBuffer,
         const uint32_t currentFrame
   ) override;
   void updateUBO(
         const VkDevice& logicalDevice,
         const uint32_t& currentFrame,
         const UBOinfo& uboInfo
   ) override;

   const std::string& getTextureFolderName() const;
   const std::shared_ptr<Texture>& getEnvMap() const;
   const std::shared_ptr<Texture>& getIrradianceMap() const;
   const std::vector<Mesh<Attributes::SKYBOX::Vertex>>& getMeshes() const;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;
   void uploadVertexData(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool
   );
   void uploadTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkSampleCountFlagBits& samplesCount,
         const std::shared_ptr<CommandPool>& commandPool,
         const VkQueue& graphicsQueue
   ) override;
   void createUniformBuffers(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   ) override;

   std::string                m_textureFolderName;
   std::shared_ptr<Texture>   m_envMap;
   std::shared_ptr<Texture>   m_irradianceMap;

   std::vector<Mesh<Attributes::SKYBOX::Vertex>> m_meshes;
};
