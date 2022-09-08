#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class DescriptorPool
{

public:

   DescriptorPool();
   ~DescriptorPool();

   void createDescriptorPool(
      const VkDevice& logicalDevice,
      const size_t size,
      const size_t maxSets
   );
   void createUniformBuffers(
      const VkPhysicalDevice physicalDevice,
      const VkDevice logicalDevice,
      const size_t nSets
   );
   void createDescriptorSetLayout(const VkDevice& logicalDevice);
   void createDescriptorSets(const VkDevice logicalDevice);

   void destroyDescriptorPool(const VkDevice& logicalDevice);
   void destroyDescriptorSetLayout(const VkDevice& logicalDevice);
   void destroyUniformBuffersAndMemories(const VkDevice& logicalDevice);

   const std::vector<VkDescriptorSet> getDescriptorSets() const;
   const VkDescriptorSetLayout getDescriptorSetLayout() const;

   void updateUniformBuffer(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent
   );

private:

   void createUboLayoutBinding(
      VkDescriptorSetLayoutBinding& uboLayoutBinding
   ); 

   void allocDescriptorSets(const VkDevice& logicalDevice);

   VkDescriptorPool             m_descriptorPool;
   VkDescriptorSetLayout        m_descriptorSetLayout;
   std::vector<VkDescriptorSet> m_descriptorSets;

   std::vector<VkBuffer>        m_uniformBuffers;
   std::vector<VkDeviceMemory>  m_uniformBuffersMemory;

};
