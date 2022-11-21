#pragma once

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>

class NormalPBR : public Model
{

public:

   NormalPBR(
         const std::string& name,
         const std::string& modelFilename,
         const glm::fvec4& pos = glm::fvec4(0.0f),
         const glm::fvec3& rot = glm::fvec3(0.0f),
         const glm::fvec3& size = glm::fvec3(1.0f)
   );

   ~NormalPBR() override;

   void destroy(const VkDevice& logicalDevice) override;

   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         const Texture& irradianceMap,
         const std::shared_ptr<ShadowMap>& shadowMap,
         DescriptorPool& descriptorPool
   );

   void createUniformBuffers(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   ) override;

   void uploadVertexData(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool
   ) override;

   void loadTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkSampleCountFlagBits& samplesCount,
         const std::shared_ptr<CommandPool>& commandPool,
         VkQueue& graphicsQueue
   ) override;

   void updateUBO(
         const VkDevice& logicalDevice,
         const glm::vec4& cameraPos,
         const glm::mat4& view,
         const glm::mat4& proj,
         const glm::mat4& lightSpace,
         const int& lightsCount,
         const std::vector<std::shared_ptr<Model>>& models,
         const uint32_t& currentFrame
   );
   void updateUBOlightsInfo(
         const VkDevice& logicalDevice,
         const std::vector<size_t> lightModelIndices,
         const std::vector<std::shared_ptr<Model>>& models,
         const uint32_t& currentFrame
   );
   const glm::mat4& getModelM() const;

         
   // Info to update UBO.
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];

   // TODO: make it private
   std::vector<Mesh<Attributes::PBR::Vertex>> m_meshes;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;
   std::string getMaterialTextureName(
      aiMaterial* material,
      const aiTextureType& type,
      const std::string& typeName,
      const std::string& defaultTextureFile
   );

   std::shared_ptr<UBO> m_uboLights;
   bool                 m_hasNormalMap;

   DescriptorTypes::UniformBufferObject::NormalPBR m_dataInShader;
   DescriptorTypes::UniformBufferObject::LightInfo m_lightsInfo[config::LIGHTS_COUNT];
};
