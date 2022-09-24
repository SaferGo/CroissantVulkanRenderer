#include <VulkanToyRenderer/Model/Model.h>

#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <VulkanToyRenderer/Model/Vertex.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>

// REMEMBER: LoadObj automatically applies triangularization by default!


/*
 * Approach taken from https://en.cppreference.com/w/cpp/utility/hash.
 * This helps us to use std::unordered_map to avoid vertex duplication.
 */
namespace std {
   template<> struct hash<Vertex>
   {
      size_t operator()(Vertex const& vertex) const
      {
         return (
               (hash<glm::vec3>()(vertex.pos) ^
               (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
               (hash<glm::vec2>()(vertex.texCoord) << 1
         );
      }
   };
};

Model::Model(
      const char* pathToMesh,
      const std::string& texture,
      const std::string& modelName
) : m_name(modelName), m_textureFileName(texture)
{
   // We'll read the texture file later(after the creation of a cmd pool and
   // the queue handles).
   // Improve this.
   m_textureFileName= texture;
   loadVertexInfo(pathToMesh);
}

void Model::updateUBO(
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject& newUbo,
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

Model::~Model() {}

void Model::destroy(const VkDevice& logicalDevice)
{
   m_ubo.destroyUniformBuffersAndMemories(logicalDevice);
   m_texture->destroyTexture(logicalDevice);
   bufferManager::destroyBuffer(
         logicalDevice,
         m_vertexBuffer
   );
   bufferManager::destroyBuffer(
         logicalDevice,
         m_indexBuffer
   );
   
   bufferManager::freeMemory(
         logicalDevice,
         m_vertexMemory
   );
   bufferManager::freeMemory(
         logicalDevice,
         m_indexMemory
   );
}


const std::string& Model::getName() const
{
   return m_name;
}

void Model::loadVertexInfo(const char* pathToMesh)
{
   // Holds all the positions, normals and texture coordinates.
   tinyobj::attrib_t attrib;
   // Contains all the objects and their faces.
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;

   std::string warn, err;

   auto status = tinyobj::LoadObj(
         &attrib,
         &shapes,
         &materials,
         &warn,
         &err,
         pathToMesh
   );

   if (!status)
      throw std::runtime_error(warn + err);

   initExtremeValues();

   std::unordered_map<Vertex, uint32_t> uniqueVertices{};

   for (const auto& shape : shapes)
   {
      for (const auto& index : shape.mesh.indices)
      {
         Vertex vertex{};

         vertex.pos = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]
         };

         // These will help us to move model to the center.
         extremeX[0] = std::fmin(vertex.pos.x, extremeX[0]);
         extremeX[1] = std::fmax(vertex.pos.x, extremeX[1]);
         extremeY[0] = std::fmin(vertex.pos.y, extremeY[0]);
         extremeY[1] = std::fmax(vertex.pos.y, extremeY[1]);
         extremeZ[0] = std::fmin(vertex.pos.z, extremeZ[0]);
         extremeZ[1] = std::fmax(vertex.pos.z, extremeZ[1]);

         vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2]
         };

         vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            // Flips the vertical component.
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
         };

         // In this case de mesh is already iluminated in the texture.
         vertex.color = {1.0f, 1.0f, 1.0f};

         // Avoids vertex duplication
         if (uniqueVertices.count(vertex) == 0)
         {
            uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
            m_vertices.push_back(vertex);
         }

         m_indices.push_back(uniqueVertices[vertex]);
      }
   }


   translateToCenter();
   
   float maxZ = std::fmax(
         std::fabs(extremeZ[1]),
         std::fabs(extremeZ[0])
   );

   // If the mesh is too big, this will scale it.
   if (maxZ > 1.0f || maxZ < -1.0f)
      makeItSmaller(maxZ);

   actualPos = glm::fvec3(
         0.0f,
         0.0f,
         0.0f
   );
   actualSize = glm::fvec3(
         1.0f,
         1.0f,
         1.0f
   );
   actualRot = glm::fvec3(
         0.0f,
         0.0f,
         0.0f
   );
}

void Model::uploadVertexData(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      VkQueue& graphicsQueue,
      CommandPool& commandPool
) {

   // Vertex Buffer(with staging buffer)
   bufferManager::createBufferAndTransferToDevice(
         commandPool,
         physicalDevice,
         logicalDevice,
         m_vertices,
         graphicsQueue,
         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         m_vertexMemory,
         m_vertexBuffer
      );

   // Index Buffer(with staging buffer)
   bufferManager::createBufferAndTransferToDevice(
         commandPool,
         physicalDevice,
         logicalDevice,
         m_indices,
         graphicsQueue,
         VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         m_indexMemory,
         m_indexBuffer
   );

}

void Model::createTexture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      CommandPool& commandPool,
      VkQueue& graphicsQueue,
      const VkFormat& format
) {
   m_texture = std::make_unique<Texture>(
         physicalDevice,
         logicalDevice,
         m_textureFileName,
         format,
         commandPool,
         graphicsQueue
   );
}

void Model::createUniformBuffers(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t& uboCount
) {
   m_ubo.createUniformBuffers(
         physicalDevice,
         logicalDevice,
         uboCount
   );
}

void Model::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool
) {
   m_descriptorSets.createDescriptorSets(
         logicalDevice,
         // List of all the descriptors
         // (improve this?)
         m_texture->getTextureImageView(),
         m_texture->getTextureSampler(),
         m_ubo.getUniformBuffers(),
         descriptorSetLayout,
         descriptorPool
   );
}

const VkDescriptorSet& Model::getDescriptorSet(const uint32_t index) const
{
   return m_descriptorSets.getDescriptorSet(index);
}

VkBuffer& Model::getVertexBuffer()
{
   return m_vertexBuffer;
}

VkBuffer& Model::getIndexBuffer()
{
   return m_indexBuffer;
}

uint32_t Model::getIndexCount()
{
   return m_indices.size();
}

void Model::initExtremeValues()
{
   extremeX[0] = extremeY[0] = extremeZ[0] = std::numeric_limits<float>::max();
   extremeX[1] = extremeY[1] = extremeZ[1] = std::numeric_limits<float>::min();

}

void Model::translateToCenter()
{
   float backX = -((extremeX[1] + extremeX[0]) / 2.0);
   float backY = -((extremeY[1] + extremeY[0]) / 2.0);
   float backZ = -((extremeZ[1] + extremeZ[0]) / 2.0);

   initExtremeValues();

   for (auto& vertex : m_vertices)
   {
      vertex.pos.x += backX;
      vertex.pos.y += backY;
      vertex.pos.z += backZ;
      extremeX[0] = std::fmin(vertex.pos.x, extremeX[0]);
      extremeX[1] = std::fmax(vertex.pos.x, extremeX[1]);
      extremeY[0] = std::fmin(vertex.pos.y, extremeY[0]);
      extremeY[1] = std::fmax(vertex.pos.y, extremeY[1]);
      extremeZ[0] = std::fmin(vertex.pos.z, extremeZ[0]);
      extremeZ[1] = std::fmax(vertex.pos.z, extremeZ[1]);

   }
}

void Model::makeItSmaller(const float maxZ)
{
   initExtremeValues();

   for (auto& vertex : m_vertices)
   {
      vertex.pos.x /= maxZ;
      vertex.pos.y /= maxZ;
      vertex.pos.z /= maxZ;
      extremeX[0] = std::fmin(vertex.pos.x, extremeX[0]);
      extremeX[1] = std::fmax(vertex.pos.x, extremeX[1]);
      extremeY[0] = std::fmin(vertex.pos.y, extremeY[0]);
      extremeY[1] = std::fmax(vertex.pos.y, extremeY[1]);
      extremeZ[0] = std::fmin(vertex.pos.z, extremeZ[0]);
      extremeZ[1] = std::fmax(vertex.pos.z, extremeZ[1]);
   }
}
