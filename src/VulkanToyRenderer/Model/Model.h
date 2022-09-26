#pragma once

#include <vector>
#include <string>
#include <array>
#include <optional>

#include <glm/glm.hpp>

#include <VulkanToyRenderer/Model/Vertex.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/UBO.h>

// REMEMBER: LoadObj automatically applies triangularization by default!

class Model
{

public:

   Model(
         const char* pathToMesh,
         const std::string& texture,
         const std::string& modelName
   );
   ~Model();
   void destroy(const VkDevice& logicalDevice);
   void updateUBO(
         const VkDevice& logicalDevice,
         DescriptorTypes::UniformBufferObject& newUbo,
         const uint32_t& currentFrame
   );
   void uploadVertexData(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         VkQueue& graphicsQueue,
         CommandPool& commandPool
   );
   void createTexture(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         CommandPool& commandPool,
         VkQueue& graphicsQueue,
         const VkFormat& format
   );
   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorPool& descriptorPool
   );

   void createUniformBuffers(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   );
         
   const std::string& getName() const;
   const VkDescriptorSet& getDescriptorSet(const uint32_t index) const;
   VkBuffer& getVertexBuffer();
   VkBuffer& getIndexBuffer();
   uint32_t getIndexCount();

   // Info to update UBO.
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];
   glm::fvec3 actualPos;
   glm::fvec3 actualSize;
   glm::fvec3 actualRot;

private:

   void loadVertexInfo(const char* pathToMesh);
   void translateToCenter();
   void initExtremeValues();
   void makeItSmaller(const float maxZ);

   std::string m_name;

   std::vector<Vertex>   m_vertices;
   std::vector<uint32_t> m_indices;


   VkBuffer       m_vertexBuffer;
   VkDeviceMemory m_vertexMemory;

   VkBuffer       m_indexBuffer;
   VkDeviceMemory m_indexMemory;

   // Texture
   std::string m_textureFileName;
   std::unique_ptr<Texture> m_texture;
   // Also Sampler from Texture obj.
   DescriptorSets m_descriptorSets;

   // Descriptors
   UBO m_ubo;
};
