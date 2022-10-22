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
#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipeline.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>

class Skybox : public Model
{

public:

   Skybox(
         const std::string& name,
         const std::string& textureFolderName
   );

   ~Skybox() override;

   void destroy(const VkDevice& logicalDevice);

   void createTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkSampleCountFlagBits& samplesCount,
         CommandPool& commandPool,
         VkQueue& graphicsQueue
   ) override;

   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorPool& descriptorPool
   ) override;

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
      CommandPool& commandPool
   );

   // Info to update UBO.
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];

   std::vector<Mesh<Attributes::SKYBOX::Vertex>> m_meshes;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;
};
