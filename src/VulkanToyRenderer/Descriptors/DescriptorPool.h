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
      const std::vector<VkDescriptorType>& descriptorTypes,
      const std::vector<uint32_t> descriptorSizes,
      const uint32_t descriptorSetsCount
   );
   void allocDescriptorSets(
         const VkDevice& logicalDevice,
         std::vector<VkDescriptorSet>& descriptorSets,
         std::vector<VkDescriptorSetLayout>& descriptorSetLayouts
   );
   void destroyDescriptorPool(const VkDevice& logicalDevice);

   void updateUniformBuffer1(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent,
         std::vector<VkDeviceMemory>& uniformBufferMemories
   );
   void updateUniformBuffer2(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent,
         std::vector<VkDeviceMemory>& uniformBufferMemories
   );


private:

   void createPoolSize(
         const size_t size,
         const VkDescriptorType& type,
         VkDescriptorPoolSize& poolSize
   );
      
   VkDescriptorPool  m_descriptorPool;
};
