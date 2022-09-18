#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class UBO
{

public:

   UBO();
   ~UBO();
   void createUniformBuffers(
         const VkPhysicalDevice physicalDevice,
         const VkDevice logicalDevice,
         const uint32_t nSets
   );
   std::vector<VkDeviceMemory>& getUniformBufferMemories();
   std::vector<VkBuffer>& getUniformBuffers();
   void destroyUniformBuffersAndMemories(const VkDevice& logicalDevice);

private:

   std::vector<VkBuffer>        m_uniformBuffers;
   std::vector<VkDeviceMemory>  m_uniformBufferMemories;

};
