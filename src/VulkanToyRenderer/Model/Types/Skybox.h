#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/Model/ModelInfo.h>
#include <VulkanToyRenderer/Texture/Type/Cubemap.h>
#include <VulkanToyRenderer/Descriptor/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptor/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptor/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>

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
   const Texture& getEnvMap() const;
   const Texture& getIrradianceMap() const;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;
   void loadIrradianceMap(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const TextureToLoadInfo& textureInfo,
         const VkSampleCountFlagBits& samplesCount,
         const std::shared_ptr<CommandPool>& commandPool,
         const VkQueue& graphicsQueue
   );
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
