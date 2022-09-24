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
      const std::vector<VkDescriptorPoolSize> poolSizes,
      const uint32_t descriptorSetsCount
   );
   const VkDescriptorPool& getDescriptorPool() const;
   void allocDescriptorSets(
         const VkDevice& logicalDevice,
         std::vector<VkDescriptorSet>& descriptorSets,
         std::vector<VkDescriptorSetLayout>& descriptorSetLayouts
   );
   void destroyDescriptorPool(const VkDevice& logicalDevice);


private:

   VkDescriptorPool  m_descriptorPool;
};
