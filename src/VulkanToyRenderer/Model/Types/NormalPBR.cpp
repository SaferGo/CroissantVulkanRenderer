#include <VulkanToyRenderer/Model/Types/NormalPBR.h>

#include <iostream>

#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>
#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Model/Types/Light.h>

NormalPBR::NormalPBR(
      const std::string& name,
      const std::string& modelFileName,
      const glm::fvec4& pos,
      const glm::fvec3& rot,
      const glm::fvec3& size
) : Model(name, ModelType::NORMAL_PBR, pos, rot, size) {

   loadModel((std::string(MODEL_DIR) + modelFileName).c_str());

   for (auto& mesh : m_meshes)
   {
      mesh.m_textures.resize(
            GRAPHICS_PIPELINE::PBR::TEXTURES_PER_MESH_COUNT
      );
   }
}

NormalPBR::~NormalPBR() {}

void NormalPBR::destroy(const VkDevice& logicalDevice)
{
   m_ubo.destroyUniformBuffersAndMemories(logicalDevice);
   m_uboLights.destroyUniformBuffersAndMemories(logicalDevice);

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
      const std::string& typeName,
      const std::string& defaultTextureFile
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
      if (typeName == "NORMALS")
         std::cout << " NSADSADSADSA NO HAY NORMALS\n";
      // TODO: Improve this
      return defaultTextureFile;
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

      if (mesh->mNormals != NULL)
      {
         vertex.normal = glm::normalize(
               glm::fvec3(
                  mesh->mNormals[i].x,
                  mesh->mNormals[i].y,
                  mesh->mNormals[i].z
               )
         );
      } else {
         std::cout << " NO HAY NORMALS !\n";
         vertex.normal = glm::fvec3(1.0f);
      }

      if (mesh->mTextureCoords[0] != NULL)
      {

         vertex.texCoord = {
            mesh->mTextureCoords[0][i].x,
            mesh->mTextureCoords[0][i].y,
         };
         newMesh.m_hasTextureCoords = true;

      } else
      {
         vertex.texCoord = glm::fvec3(1.0f);
         newMesh.m_hasTextureCoords = false;
      }

      if (mesh->mTangents != NULL)
      {
         vertex.tangent = glm::normalize(
               glm::fvec3(
                  mesh->mTangents[i].x,
                  mesh->mTangents[i].y,
                  mesh->mTangents[i].z
               )
         );
      } else
         vertex.tangent = glm::fvec3(1.0f);

      //glm::vec3 bitangent = glm::cross(vertex.normal, tangent);
      
      newMesh.m_vertices.push_back(vertex);

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

      TextureToLoadInfo info;

      info.name = getMaterialTextureName(
            material,
            aiTextureType_DIFFUSE,
            "DIFFUSE",
            "textures/default.png"
      );
      info.format = VK_FORMAT_R8G8B8A8_SRGB;

      newMesh.m_texturesToLoadInfo.push_back(info);


      info.name = getMaterialTextureName(
            material,
            aiTextureType_UNKNOWN,
            "METALIC_ROUGHNESS",
            "textures/default.png"
      );
      info.format = VK_FORMAT_R8G8B8A8_SRGB;

      newMesh.m_texturesToLoadInfo.push_back(info);

      info.name = getMaterialTextureName(
            material,
            aiTextureType_NORMALS,
            "NORMALS",
            "textures/default.png"
      );
      info.format = VK_FORMAT_R8G8B8A8_UNORM;

      newMesh.m_texturesToLoadInfo.push_back(info);
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
   m_uboLights.createUniformBuffers(
         physicalDevice,
         logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::LightInfo) * 10
   );
}

void NormalPBR::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool
) {
   std::vector<UBO*> opUBOs = {&m_ubo, &m_uboLights};

   for (auto& mesh : m_meshes)
   {
      mesh.m_descriptorSets = DescriptorSets(
            logicalDevice,
            GRAPHICS_PIPELINE::PBR::UBOS_INFO,
            GRAPHICS_PIPELINE::PBR::SAMPLERS_INFO,
            mesh.m_textures,
            opUBOs,
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
      const VkSampleCountFlagBits& samplesCount,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
) {
   for (auto& mesh : m_meshes)
   {
      // TODO: Improve this.
      if (!mesh.m_hasTextureCoords)
         continue;

      for (size_t i = 0; i < mesh.m_textures.size(); i++)
      {
         mesh.m_textures[i] = Texture(
               physicalDevice,
               logicalDevice,
               mesh.m_texturesToLoadInfo[i],
               // isSkybox
               false,
               samplesCount,
               commandPool,
               graphicsQueue
         );
      }
   }
}

void NormalPBR::updateUBO(
      const VkDevice& logicalDevice,
      const glm::vec4& cameraPos,
      const glm::mat4& view,
      const glm::mat4& proj,
      const int& lightsCount,
      const std::vector<std::shared_ptr<Model>>& models,
      const uint32_t& currentFrame
) {

   m_basicInfo.model = UBOutils::getUpdatedModelMatrix(
         m_pos,
         m_rot,
         m_size
   );
   m_basicInfo.view = view;
   m_basicInfo.proj = proj;

   m_basicInfo.cameraPos = cameraPos;
   m_basicInfo.lightsCount = lightsCount;

   size_t size = sizeof(m_basicInfo);
   UBOutils::updateUBO(logicalDevice, m_ubo, size, &m_basicInfo, currentFrame);
}

void NormalPBR::updateUBOlightsInfo(
      const VkDevice& logicalDevice,
      const std::vector<size_t> lightModelIndices,
      const std::vector<std::shared_ptr<Model>>& models,
      const uint32_t& currentFrame
) {
   for (size_t i = 0; i < lightModelIndices.size(); i++)
   {
      size_t j = lightModelIndices[i];

      if (auto pModel = std::dynamic_pointer_cast<Light>(models[j]))
      {
         m_lightsInfo[i].pos = pModel->getPos();
         m_lightsInfo[i].color = pModel->getColor();
         m_lightsInfo[i].attenuation = pModel->getAttenuation();
         m_lightsInfo[i].radius = pModel->getRadius();
         m_lightsInfo[i].type = (int)pModel->getLightType();
      }
   }

   size_t size = sizeof(m_lightsInfo[0]) * 10;

   UBOutils::updateUBO(
         logicalDevice,
         m_uboLights,
         size,
         &m_lightsInfo,
         currentFrame
   );
}
