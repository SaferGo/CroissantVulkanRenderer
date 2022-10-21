#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <VulkanToyRenderer/Model/Mesh.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/DescriptorInfo.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>

enum class ModelType
{
   NONE = 0,
   NORMAL_PBR = 1,
   DIRECTIONAL_LIGHT  = 2,
   SKYBOX = 3
};

class Model
{

public:

   Model(
         const std::string& name,
         const ModelType& type,
         const glm::fvec4& pos = glm::fvec4(0.0f),
         const glm::fvec3& rot = glm::fvec3(0.0f),
         const glm::fvec3& size = glm::fvec3(1.0f)
   );

   virtual ~Model() = 0;

   virtual void destroy(const VkDevice& logicalDevice) = 0;

   virtual void uploadVertexData(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         VkQueue& graphicsQueue,
         CommandPool& commandPool
   ) = 0;

   virtual void createTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         CommandPool& commandPool,
         VkQueue& graphicsQueue
   ) = 0;
   virtual void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorPool& descriptorPool
   ) = 0;

   virtual void createUniformBuffers(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   ) = 0;
         
   const std::string& getName() const;
   const ModelType& getType() const;
   const glm::fvec4& getPos() const;
   const glm::fvec3& getRot() const;
   const glm::fvec3& getSize() const;
   void setPos(const glm::fvec4& newPos);
   void setRot(const glm::fvec3& newRot);
   void setSize(const glm::fvec3& newSize);

   // Info to update UBO.
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];

protected:

   virtual void processMesh(aiMesh* mesh, const aiScene* scene) = 0;
   void loadModel(const char* pathToModel);

   std::string m_name;
   ModelType m_type;
   UBO m_ubo;

   glm::fvec4 m_pos;
   glm::fvec3 m_rot;
   glm::fvec3 m_size;


private:

   void processNode(aiNode* node, const aiScene* scene);

};
