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
   unsigned int flags;

   if (m_type == ModelType::NORMAL_PBR)
   {
      flags = (
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace
      );
   } else
   {
      flags = (
            aiProcess_Triangulate |
            aiProcess_FlipUVs
      );
   }

   Assimp::Importer importer;
   auto* scene = importer.ReadFile(
         pathToModel,
         flags
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

