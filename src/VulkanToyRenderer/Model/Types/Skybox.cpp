#include <VulkanToyRenderer/Model/Types/Skybox.h>

#include <string>
#include <vector>
#include <cstring>
#include <iostream>

#include <vulkan/vulkan.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Descriptors/descriptorSetLayoutUtils.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Pipeline/Graphics.h>
#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/BufferManager/bufferManager.h>
#include <VulkanToyRenderer/Math/mathUtils.h>

Skybox::Skybox(
      const std::string& name,
      const std::string& textureFolderName
) : Model(name, ModelType::SKYBOX), m_textureFolderName(textureFolderName)
{
   loadModel((std::string(MODEL_DIR) + "Cube.gltf").c_str());
}

void Skybox::destroy(const VkDevice& logicalDevice)
{
   m_ubo->destroy();

   for (auto& texture : m_texturesLoaded)
      texture->destroy();
   m_irradianceMap->destroy();

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

void Skybox::processMesh(aiMesh* mesh, const aiScene* scene)
{
   Mesh<Attributes::SKYBOX::Vertex> newMesh;

   for (size_t i = 0; i < mesh->mNumVertices; i++)
   {
      Attributes::SKYBOX::Vertex vertex{};
   
      vertex.pos = {
         mesh->mVertices[i].x,
         mesh->mVertices[i].y,
         mesh->mVertices[i].z
      };
   
      newMesh.vertices.push_back(vertex);
   }
   
   for (size_t i = 0; i < mesh->mNumFaces; i++)
   {
      auto face = mesh->mFaces[i];

      for (size_t j = 0; j < face.mNumIndices; j++)
         newMesh.indices.push_back(face.mIndices[j]);
   }

   m_meshes.push_back(newMesh);
}

Skybox::~Skybox() {}

void Skybox::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool
) {

   std::vector<UBO*> opUBOs = {m_ubo.get()};

   for (auto& mesh : m_meshes)
   {
      mesh.descriptorSets = DescriptorSets(
            logicalDevice,
            GRAPHICS_PIPELINE::SKYBOX::UBOS_INFO,
            GRAPHICS_PIPELINE::SKYBOX::SAMPLERS_INFO,
            mesh.textures,
            opUBOs,
            descriptorSetLayout,
            descriptorPool
      );
   }
}

void Skybox::createUniformBuffers(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t& uboCount
) {
   m_ubo = std::make_shared<UBO>(
         physicalDevice,
         logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::Skybox)
   );
}

void Skybox::uploadVertexData(
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

void Skybox::loadTextures(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
) {

   const size_t nTextures = GRAPHICS_PIPELINE::SKYBOX::TEXTURES_PER_MESH_COUNT;
   TextureToLoadInfo info = {
      m_name,
      VK_FORMAT_R32G32B32A32_SFLOAT
   };

   for (auto& mesh : m_meshes)
   {
      for (size_t i = 0; i < nTextures; i++)
      {
         auto it = (
               m_texturesID.find(info.name)
         );

         if (it == m_texturesID.end())
         {
            mesh.textures.push_back(
                  std::make_shared<Texture>(
                     physicalDevice,
                     logicalDevice,
                     false,
                     info,
                     m_textureFolderName,
                     samplesCount,
                     commandPool,
                     graphicsQueue
                  )
            );

            m_texturesLoaded.push_back(mesh.textures[i]);
            m_texturesID[info.name] = (
                  m_texturesLoaded.size() - 1
            );
         } else
            mesh.textures.push_back(m_texturesLoaded[it->second]);
      }
   }

   info = {
      "Irradiance.hdr",
      VK_FORMAT_R32G32B32A32_SFLOAT
   };
   loadIrradianceMap(
         physicalDevice,
         logicalDevice,
         info,
         samplesCount,
         commandPool,
         graphicsQueue
   );
}

void Skybox::loadIrradianceMap(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue
) {
   m_irradianceMap = std::make_shared<Texture>(
         physicalDevice,
         logicalDevice,
         true,
         textureInfo,
         m_textureFolderName,
         samplesCount,
         commandPool,
         graphicsQueue
   );
}

void Skybox::updateUBO(
      const VkDevice& logicalDevice,
      const glm::vec4& cameraPos,
      const glm::mat4& view,
      const VkExtent2D&  extent,
      const uint32_t& currentFrame
) {

   DescriptorTypes::UniformBufferObject::Skybox newUBO;
   
   newUBO.model = glm::translate(
      glm::mat4(1.0f),
      glm::vec3(cameraPos)
   );

   newUBO.view = view;
   
   newUBO.proj = mathUtils::getUpdatedProjMatrix(
      // Different to the original
      glm::radians(75.0f),
      extent.width / (float)extent.height,
      0.01f,
      40.0f
   );
   
   const size_t size = sizeof(newUBO);
   UBOutils::updateUBO(logicalDevice, m_ubo, size, &newUBO, currentFrame);
}

const Texture& Skybox::getIrradianceMap() const
{
   return *m_irradianceMap;
}
