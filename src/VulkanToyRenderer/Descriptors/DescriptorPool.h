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
   void createDescriptorSets(
         const VkDevice logicalDevice,
         const VkImageView& textureImageView,
         const VkSampler& textureSampler,
         std::vector<VkDescriptorSet>& descriptorSets
   );

   void destroyDescriptorPool(const VkDevice& logicalDevice);
   void destroyDescriptorSetLayout(const VkDevice& logicalDevice);
   void destroyUniformBuffersAndMemories(const VkDevice& logicalDevice);

   const VkDescriptorSetLayout getDescriptorSetLayout() const;

   void updateUniformBuffer1(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent
   );
   void updateUniformBuffer2(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent
   );


private:

   void allocDescriptorSets(
         const VkDevice& logicalDevice,
         std::vector<VkDescriptorSet>& descriptorSets
   );

   VkDescriptorPool             m_descriptorPool;
   VkDescriptorSetLayout        m_descriptorSetLayout;

   // Types of descriptors
   std::vector<VkBuffer>        m_uniformBuffers;
   std::vector<VkDeviceMemory>  m_uniformBuffersMemory;

};
