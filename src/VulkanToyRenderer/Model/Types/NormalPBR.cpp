#include <VulkanToyRenderer/Model/Types/NormalPBR.h>

#include <iostream>

#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>
#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/BufferManager/bufferManager.h>
#include <VulkanToyRenderer/Model/Types/Light.h>
#include <VulkanToyRenderer/Math/mathUtils.h>

NormalPBR::NormalPBR(
      const std::string& name,
      const std::string& modelFileName,
      const glm::fvec4& pos,
      const glm::fvec3& rot,
      const glm::fvec3& size
) : Model(name, ModelType::NORMAL_PBR, pos, rot, size) {

   loadModel((std::string(MODEL_DIR) + modelFileName).c_str());
}

NormalPBR::~NormalPBR() {}

void NormalPBR::destroy(const VkDevice& logicalDevice)
{
   m_ubo->destroy();
   m_uboLights->destroy();

   for (auto& texture : m_texturesLoaded) 
      texture->destroy();

   for (auto& mesh : m_meshes)
   {

      bufferManager::destroyBuffer(
            logicalDevice,
            mesh.vertexBuffer
      );
      bufferManager::destroyBuffer(
            logicalDevice,
            mesh.indexBuffer
      );
      
      bufferManager::freeMemory(
            logicalDevice,
            mesh.vertexMemory
      );
      bufferManager::freeMemory(
            logicalDevice,
            mesh.indexMemory
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
      // TODO: manage when there are several textures of the same type.

      //for (size_t i = 0; i < material->GetTextureCount(type); i++)
      //{
      //   aiString str;
      //   material->GetTexture(type, i, &str);
      //}
      aiString str;
      material->GetTexture(type, 0, &str);

      return str.C_Str();

   } else
      return defaultTextureFile;
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
      } else
         throw std::runtime_error("Mesh doesn't have normals!");

      if (mesh->mTextureCoords[0] != NULL)
      {

         vertex.texCoord = {
            mesh->mTextureCoords[0][i].x,
            mesh->mTextureCoords[0][i].y,
         };

      } else
         vertex.texCoord = glm::fvec3(1.0f);

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

      vertex.posInLightSpace = glm::fvec4(1.0f);
      
      newMesh.vertices.push_back(vertex);

   }

   for (size_t i = 0; i < mesh->mNumFaces; i++)
   {
      auto face = mesh->mFaces[i];
      for (size_t j = 0; j < face.mNumIndices; j++)
         newMesh.indices.push_back(face.mIndices[j]);
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

      newMesh.texturesToLoadInfo.push_back(info);

      info.name = getMaterialTextureName(
            material,
            aiTextureType_UNKNOWN,
            "METALIC_ROUGHNESS",
            "textures/default.png"
      );
      info.format = VK_FORMAT_R8G8B8A8_SRGB;

      newMesh.texturesToLoadInfo.push_back(info);

      info.name = getMaterialTextureName(
            material,
            aiTextureType_NORMALS,
            "NORMALS",
            "textures/default.png"
      );
      info.format = VK_FORMAT_R8G8B8A8_UNORM;

      if (info.name == "textures/default.png")
         m_hasNormalMap = false;
      else
         m_hasNormalMap = true;


      newMesh.texturesToLoadInfo.push_back(info);
   }

   m_meshes.push_back(newMesh);
}

void NormalPBR::createUniformBuffers(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t& uboCount
) {
   m_ubo = std::make_shared<UBO>(
         physicalDevice,
         logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::NormalPBR)
   );
   m_uboLights = std::make_shared<UBO>(
         physicalDevice,
         logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::LightInfo) * 10
   );
}

void NormalPBR::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      const Texture& irradianceMap,
      const std::shared_ptr<ShadowMap>& shadowMap,
      DescriptorPool& descriptorPool
) {
   std::vector<UBO*> opUBOs = {
      (m_ubo.get()),
      (m_uboLights.get())
   };

  for (auto& mesh : m_meshes)
   {
      mesh.descriptorSets = DescriptorSets(
            logicalDevice,
            GRAPHICS_PIPELINE::PBR::UBOS_INFO,
            GRAPHICS_PIPELINE::PBR::SAMPLERS_INFO,
            mesh.textures,
            opUBOs,
            descriptorSetLayout,
            descriptorPool,
            std::make_optional(irradianceMap),
            std::make_optional(shadowMap->getShadowMapView()),
            std::make_optional(shadowMap->getSampler())
      );
   }

}

void NormalPBR::uploadVertexData(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool
) {

   for (auto& mesh : m_meshes)
   {
      // Vertex Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            commandPool,
            physicalDevice,
            logicalDevice,
            mesh.vertices.data(),
            sizeof(mesh.vertices[0]) * mesh.vertices.size(),
            graphicsQueue,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            mesh.vertexMemory,
            mesh.vertexBuffer
         );

      // Index Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            commandPool,
            physicalDevice,
            logicalDevice,
            mesh.indices.data(),
            sizeof(mesh.indices[0]) * mesh.indices.size(),
            graphicsQueue,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            mesh.indexMemory,
            mesh.indexBuffer
      );
   }
}

/*
 * Creates and loads all the samplers used in the shader of each mesh.
 */
void NormalPBR::loadTextures(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
) {

   const size_t nTextures = GRAPHICS_PIPELINE::PBR::TEXTURES_PER_MESH_COUNT;

   for (auto& mesh : m_meshes)
   {
      // Samplers of textures.
      for (size_t i = 0; i < nTextures; i++)
      {
         auto it = (
               m_texturesID.find(mesh.texturesToLoadInfo[i].name)
         );

         if (it == m_texturesID.end())
         {
            mesh.textures.push_back(
                  std::make_shared<Texture>(
                     physicalDevice,
                     logicalDevice,
                     mesh.texturesToLoadInfo[i],
                     samplesCount,
                     commandPool,
                     graphicsQueue
                  )
            );

            m_texturesLoaded.push_back(mesh.textures[i]);
            m_texturesID[mesh.texturesToLoadInfo[i].name] = (
                  m_texturesLoaded.size() - 1
            );
         } else
            mesh.textures.push_back(m_texturesLoaded[it->second]);
      }
   }
}

const glm::mat4& NormalPBR::getModelM() const
{
   return m_dataInShader.model;
}

void NormalPBR::updateUBO(
      const VkDevice& logicalDevice,
      const glm::vec4& cameraPos,
      const glm::mat4& view,
      const glm::mat4& proj,
      const glm::mat4& lightSpace,
      const int& lightsCount,
      const std::vector<std::shared_ptr<Model>>& models,
      const uint32_t& currentFrame
) {

   m_dataInShader.model = mathUtils::getUpdatedModelMatrix(
         m_pos,
         m_rot,
         m_size
   );
   m_dataInShader.view = view;
   m_dataInShader.proj = proj;
   m_dataInShader.lightSpace = lightSpace;

   m_dataInShader.cameraPos = cameraPos;
   m_dataInShader.lightsCount = lightsCount;
   m_dataInShader.hasNormalMap = m_hasNormalMap;

   size_t size = sizeof(m_dataInShader);
   UBOutils::updateUBO(
         logicalDevice,
         m_ubo,
         size,
         &m_dataInShader,
         currentFrame
   );
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
         m_lightsInfo[i].dir = pModel->getTargetPos() - pModel->getPos();
         m_lightsInfo[i].color = pModel->getColor();
         m_lightsInfo[i].attenuation = pModel->getAttenuation();
         m_lightsInfo[i].radius = pModel->getRadius();
         m_lightsInfo[i].intensity = pModel->getIntensity();
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
