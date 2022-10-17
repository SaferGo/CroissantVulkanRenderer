#include <VulkanToyRenderer/Model/Model.h>

#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>

Model::Model(
      const std::string& name,
      const ModelType& type
) : m_name(name), m_type(type) {}

template<typename T>
void Model::updateUBO(
      const VkDevice& logicalDevice,
      T& newUbo,
      const uint32_t& currentFrame
) {
   void* data;
   vkMapMemory(
         logicalDevice,
         m_ubo.getUniformBufferMemory(currentFrame),
         0,
         sizeof(newUbo),
         0,
         &data
   );
      memcpy(data, &newUbo, sizeof(newUbo));
   vkUnmapMemory(
         logicalDevice,
         m_ubo.getUniformBufferMemory(currentFrame)
   );
}
//////////////////////////////////Instances////////////////////////////////////
template void Model::updateUBO<DescriptorTypes::UniformBufferObject::NormalPBR>(
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject::NormalPBR& newUbo,
      const uint32_t& currentFrame
);
template void Model::updateUBO<DescriptorTypes::UniformBufferObject::Light>(
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject::Light& newUbo,
      const uint32_t& currentFrame
);
template void Model::updateUBO<DescriptorTypes::UniformBufferObject::Skybox>(
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject::Skybox& newUbo,
      const uint32_t& currentFrame
);
///////////////////////////////////////////////////////////////////////////////

Model::~Model() {}

const std::string& Model::getName() const
{
   return m_name;
}

const ModelType& Model::getType() const
{
   return m_type;
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
   // Processes all the node's meshes(if any).
   for (size_t i = 0; i < node->mNumMeshes; i++)
   {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      processMesh(mesh, scene);
   }

   // Processes all the node's childrens(if any).
   for (size_t i = 0; i < node->mNumChildren; i++)
      processNode(node->mChildren[i], scene);
}

void Model::loadModel(const char* pathToModel)
{
   Assimp::Importer importer;
   const auto* scene = importer.ReadFile(
         pathToModel,
         aiProcess_Triangulate | aiProcess_FlipUVs
   );

   if (!scene ||
       scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
       !scene->mRootNode
   ) {
      throw std::runtime_error(
            "ERROR::ASSIMP::" +
            std::string(importer.GetErrorString())
      );
   }

   processNode(scene->mRootNode, scene);
}

