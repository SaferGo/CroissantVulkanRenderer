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
      const ModelType& type,
      const glm::fvec4& pos,
      const glm::fvec3& rot,
      const glm::fvec3& size
) : m_type(type),
    m_name(name),
    m_pos(pos),
    m_rot(rot),
    m_size(size),
    m_hideStatus(false)
{}

Model::~Model() {}

const std::string& Model::getName() const
{
   return m_name;
}

const ModelType& Model::getType() const
{
   return m_type;
}

const bool Model::isHided() const
{
   return m_hideStatus;
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

const glm::fvec4& Model::getPos() const
{
   return m_pos;
}

const glm::fvec3& Model::getRot() const
{
   return m_rot;
}

const glm::fvec3& Model::getSize() const
{
   return m_size;
}

void Model::setPos(const glm::fvec4& newPos)
{
   m_pos = newPos;
}

void Model::setRot(const glm::fvec3& newRot)
{
   m_rot = newRot;
}

void Model::setSize(const glm::fvec3& newSize)
{
   m_size = newSize;
}

void Model::setHideStatus(const bool status)
{
   m_hideStatus = status;
}
