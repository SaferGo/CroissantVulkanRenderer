#include <VulkanToyRenderer/Model/Types/NormalPBR.h>

#include <iostream>

#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>

NormalPBR::NormalPBR(
      const std::string& name,
      const std::string& modelFileName
) : Model(name, ModelType::NORMAL_PBR) {

   loadModel((std::string(MODEL_DIR) + modelFileName).c_str());

   for (auto& mesh : m_meshes)
   {
      mesh.m_textures.resize(GRAPHICS_PIPELINE::PBR::TEXTURES_PER_MESH_COUNT);
   }

   // TODO: Load scene settings.
   actualPos = glm::fvec4(0.0f);
   actualSize = glm::fvec3(1.0f);
   actualRot = glm::fvec3(0.0f);
}

NormalPBR::~NormalPBR() {}

void NormalPBR::destroy(const VkDevice& logicalDevice)
{
   m_ubo.destroyUniformBuffersAndMemories(logicalDevice);

   for (auto& mesh : m_meshes)
   {
      for (auto& texture : mesh.m_textures) 
         texture.destroyTexture(logicalDevice);

      bufferManager::destroyBuffer(
            logicalDevice,
            mesh.m_vertexBuffer
      );
      bufferManager::destroyBuffer(
            logicalDevice,
            mesh.m_indexBuffer
      );
      
      bufferManager::freeMemory(
            logicalDevice,
            mesh.m_vertexMemory
      );
      bufferManager::freeMemory(
            logicalDevice,
            mesh.m_indexMemory
      );
   }
}

std::string NormalPBR::getMaterialTextureName(
      aiMaterial* material,
      const aiTextureType& type,
      const std::string& typeName
) {
   if (material->GetTextureCount(type) > 0)
   {
      // TODO: do something when there are many textures of the same type.

      //for (size_t i = 0; i < material->GetTextureCount(type); i++)
      //{
      //   aiString str;
      //   material->GetTexture(type, i, &str);
      //}
      aiString str;
      material->GetTexture(type, 0, &str);

      return str.C_Str();

   } else {
      // TODO!
      return "textures/default.png";
   }
}

void NormalPBR::processMesh(aiMesh* mesh, const aiScene* scene)
{
   Mesh<Attributes::PBR::Vertex> newMesh;

   for (size_t i = 0; i < mesh->mNumVertices; i++)
   {
      Attributes::PBR::Vertex vertex{};

      vertex.pos = {
         mesh->mVertices[i].x,
         mesh->mVertices[i].y,
         mesh->mVertices[i].z
      };

      vertex.normal = {
         mesh->mNormals[i].x,
         mesh->mNormals[i].y,
         mesh->mNormals[i].z
      };

      vertex.texCoord = {
         mesh->mTextureCoords[0][i].x,
         mesh->mTextureCoords[0][i].y,
      };

      newMesh.m_vertices.push_back(vertex);

      // TODO: move and scale the mesh.
   }

   for (size_t i = 0; i < mesh->mNumFaces; i++)
   {
      auto face = mesh->mFaces[i];
      for (size_t j = 0; j < face.mNumIndices; j++)
         newMesh.m_indices.push_back(face.mIndices[j]);
   }


   if (mesh->mMaterialIndex >= 0)
   {
      aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

      newMesh.m_textureNames.push_back(
            getMaterialTextureName(material, aiTextureType_DIFFUSE, "DIFFUSE")
      );
   }

   m_meshes.push_back(newMesh);
}

void NormalPBR::createUniformBuffers(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t& uboCount
) {
   m_ubo.createUniformBuffers(
         physicalDevice,
         logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::NormalPBR)
   );
}

void NormalPBR::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool
) {
   for (auto& mesh : m_meshes)
   {
      mesh.m_descriptorSets.createDescriptorSets(
            logicalDevice,
            GRAPHICS_PIPELINE::PBR::UBOS_INFO,
            GRAPHICS_PIPELINE::PBR::SAMPLERS_INFO,
            mesh.m_textures,
            m_ubo.getUniformBuffers(),
            descriptorSetLayout,
            descriptorPool
      );
   }
}

void NormalPBR::uploadVertexData(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      VkQueue& graphicsQueue,
      CommandPool& commandPool
) {

   for (auto& mesh : m_meshes)
   {
      // Vertex Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            commandPool,
            physicalDevice,
            logicalDevice,
            mesh.m_vertices.data(),
            sizeof(mesh.m_vertices[0]) * mesh.m_vertices.size(),
            graphicsQueue,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            mesh.m_vertexMemory,
            mesh.m_vertexBuffer
         );

      // Index Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            commandPool,
            physicalDevice,
            logicalDevice,
            mesh.m_indices.data(),
            sizeof(mesh.m_indices[0]) * mesh.m_indices.size(),
            graphicsQueue,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            mesh.m_indexMemory,
            mesh.m_indexBuffer
      );
   }
}

void NormalPBR::createTextures(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      CommandPool& commandPool,
      VkQueue& graphicsQueue,
      const VkFormat& format
) {
   for (auto& mesh : m_meshes)
   {
      for (size_t i = 0; i < mesh.m_textures.size(); i++)
      {
         mesh.m_textures[i] = Texture(
               physicalDevice,
               logicalDevice,
               mesh.m_textureNames[i],
               format,
               // isSkybox
               false,
               commandPool,
               graphicsQueue
         );
      }
   }
}
