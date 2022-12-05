#pragma once

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Model/ModelInfo.h>
#include <VulkanToyRenderer/Descriptor/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>

class NormalPBR : public Model
{

public:

   NormalPBR(ModelInfo& modelInfo);

   ~NormalPBR() override;

   void destroy(const VkDevice& logicalDevice) override;

   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorSetInfo* info,
         DescriptorPool& descriptorPool
   ) override;
   void bindData(
         const Graphics& graphicsPipeline,
         const VkCommandBuffer& commandBuffer,
         const uint32_t currentFrame
   ) override;
   void updateUBO(
         const VkDevice& logicalDevice,
         const uint32_t& currentFrame,
         const UBOinfo& uboInfo
   ) override;
   void updateUBOlights(
         const VkDevice& logicalDevice,
         const std::vector<size_t> lightModelIndices,
         const std::vector<std::shared_ptr<Model>>& models,
         const uint32_t& currentFrame
   );

   const glm::mat4& getModelM() const;
   const std::vector<Mesh<Attributes::PBR::Vertex>>& getMeshes() const;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;
   std::string getMaterialTextureName(
      aiMaterial* material,
      const aiTextureType& type,
      const std::string& typeName,
      const std::string& defaultTextureFile
   );
   void uploadVertexData(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool
   ) override;
   void uploadTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkSampleCountFlagBits& samplesCount,
         const std::shared_ptr<CommandPool>& commandPool,
         VkQueue& graphicsQueue
   ) override;
   void createUniformBuffers(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   ) override;

   std::shared_ptr<UBO> m_uboLights;
   bool                 m_hasNormalMap;

   DescriptorTypes::UniformBufferObject::NormalPBR m_dataInShader;
   DescriptorTypes::UniformBufferObject::LightInfo m_lightsInfo[config::LIGHTS_COUNT];
   std::vector<Mesh<Attributes::PBR::Vertex>> m_meshes;
};
