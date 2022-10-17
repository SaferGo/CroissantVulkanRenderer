#pragma once

#include <VulkanToyRenderer/Model/Model.h>

class NormalPBR : public Model
{

public:

   NormalPBR(
         const std::string& name,
         const std::string& modelFilename
   );

   ~NormalPBR() override;

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
         VkQueue& graphicsQueue,
         const VkFormat& format
   ) override;
         
   // Info to update UBO.
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];
   glm::fvec4 actualPos;
   glm::fvec3 actualSize;
   glm::fvec3 actualRot;

   std::vector<Mesh<Attributes::PBR::Vertex>> m_meshes;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;
   std::string getMaterialTextureName(
      aiMaterial* material,
      const aiTextureType& type,
      const std::string& typeName
   );

};
