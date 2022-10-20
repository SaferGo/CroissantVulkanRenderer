#pragma once

#include <VulkanToyRenderer/Model/Model.h>

class DirectionalLight : public Model
{

public:

   DirectionalLight(
         const std::string& name,
         const std::string& modelFilename
   );

   ~DirectionalLight() override;

   void destroy(const VkDevice& logicalDevice) override;

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

   void uploadVertexData(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         VkQueue& graphicsQueue,
         CommandPool& commandPool
   ) override;

   void createTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         CommandPool& commandPool,
         VkQueue& graphicsQueue
   ) override;

   void updateUBO(
         const VkDevice& logicalDevice,
         const glm::vec4& cameraPos,
         const glm::mat4& proj,
         const uint32_t& currentFrame
   );
         
   // Info to update UBO.
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];
   glm::fvec4 actualPos;
   glm::fvec3 actualSize;
   glm::fvec3 actualRot;
   glm::fvec4 color;

   std::vector<Mesh<Attributes::LIGHT::Vertex>> m_meshes;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;
};
