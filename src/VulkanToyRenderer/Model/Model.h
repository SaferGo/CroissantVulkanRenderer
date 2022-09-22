#pragma once

#include <vector>
#include <string>
#include <array>

#include <VulkanToyRenderer/Model/Vertex.h>
#include <VulkanToyRenderer/Textures/Texture.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/UBO.h>

// REMEMBER: LoadObj automatically applies triangularization by default!

struct Model
{
   Model(const char* pathToMesh, const std::string& texture);
   void loadVertexInfo(const char* pathToMesh);
   void createTexture(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkFormat& format,
         CommandPool& commandPool,
         VkQueue& graphicsQueue
   );

   const VkDescriptorSet& getDescriptorSet(const uint32_t index) const;


   std::vector<Vertex>   vertices;
   std::vector<uint32_t> indices;
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];

   VkBuffer       vertexBuffer;
   VkDeviceMemory vertexMemory;

   VkBuffer       indexBuffer;
   VkDeviceMemory indexMemory;

   // Texture
   Texture texture;
   std::string textureFile;

   // Descriptors
   UBO ubo;

   DescriptorSets descriptorSets;
};
