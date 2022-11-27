#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>

class Skybox : public Model
{

public:

   Skybox(
         const std::string& name,
         const std::string& textureFolderName
   );
   ~Skybox() override;
   void destroy(const VkDevice& logicalDevice);
   void loadTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkSampleCountFlagBits& samplesCount,
         const std::shared_ptr<CommandPool>& commandPool,
         VkQueue& graphicsQueue
   ) override;
   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorPool& descriptorPool
   );
   void createUniformBuffers(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   ) override;
   void updateUBO(
         const VkDevice& logicalDevice,
         const glm::vec4& cameraPos,
         const glm::mat4& view,
         const VkExtent2D&  extent,
         const uint32_t& currentFrame
   );
   void uploadVertexData(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool
   );
   const Texture& getIrradianceMap() const;

   // Info to update UBO.
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];

   std::vector<Mesh<Attributes::SKYBOX::Vertex>> m_meshes;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;
   void loadIrradianceMap(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const TextureToLoadInfo& textureInfo,
         const VkSampleCountFlagBits& samplesCount,
         const std::shared_ptr<CommandPool>& commandPool,
         VkQueue& graphicsQueue
   );

   std::string                m_textureFolderName;
   std::shared_ptr<Texture>   m_irradianceMap;
};
