#include <VulkanToyRenderer/Model/Types/Skybox.h>

#include <string>
#include <vector>
#include <cstring>
#include <iostream>

#include <vulkan/vulkan.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Descriptors/descriptorSetLayoutUtils.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipeline.h>
#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>

Skybox::Skybox(
      const std::string& name,
      const std::string& textureFolderName
) : Model(name, ModelType::SKYBOX)
{
   loadModel((std::string(MODEL_DIR) + "Cube.gltf").c_str());

   // It just contain one mesh with one texture(cubemap) but the for-loop is
   // just to respect consistency between the children of Model's class.
   for (auto& mesh : m_meshes)
   {
      mesh.m_textures.resize(
            GRAPHICS_PIPELINE::SKYBOX::TEXTURES_PER_MESH_COUNT
      );
      mesh.m_texturesToLoadInfo.push_back(
            {
               textureFolderName,
               VK_FORMAT_R8G8B8A8_SRGB
            }
      );
   }
}

void Skybox::destroy(const VkDevice& logicalDevice)
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
   
      newMesh.m_vertices.push_back(vertex);
   }
   
   for (size_t i = 0; i < mesh->mNumFaces; i++)
   {
      auto face = mesh->mFaces[i];

      for (size_t j = 0; j < face.mNumIndices; j++)
         newMesh.m_indices.push_back(face.mIndices[j]);
   }

   m_meshes.push_back(newMesh);
}

Skybox::~Skybox() {}

void Skybox::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool
) {
   for (auto& mesh : m_meshes)
   {
      mesh.m_descriptorSets.createDescriptorSets(
            logicalDevice,
            GRAPHICS_PIPELINE::SKYBOX::UBOS_INFO,
            GRAPHICS_PIPELINE::SKYBOX::SAMPLERS_INFO,
            mesh.m_textures,
            m_ubo.getUniformBuffers(),
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
   m_ubo.createUniformBuffers(
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

void Skybox::createTextures(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkSampleCountFlagBits& samplesCount,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
) {
   for (auto& mesh : m_meshes)
   {
      for (size_t i = 0; i < mesh.m_textures.size(); i++)
      {
         mesh.m_textures[i] = Texture(
               physicalDevice,
               logicalDevice,
               mesh.m_texturesToLoadInfo[i],
               // isSkybox
               true,
               samplesCount,
               commandPool,
               graphicsQueue
         );
      }
   }
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
   
   newUBO.proj = UBOutils::getUpdatedProjMatrix(
      // Different to the original
      glm::radians(75.0f),
      extent.width / (float)extent.height,
      0.01f,
      40.0f
   );
   
   UBOutils::updateUBO(m_ubo, logicalDevice, newUBO, currentFrame);
}


