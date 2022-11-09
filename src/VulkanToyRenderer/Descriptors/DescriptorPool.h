#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class DescriptorPool
{

public:

   DescriptorPool();
   DescriptorPool(
      const VkDevice& logicalDevice,
      const std::vector<VkDescriptorPoolSize> poolSizes,
      const uint32_t descriptorSetsCount
   );
   ~DescriptorPool();
   const VkDescriptorPool& getDescriptorPool() const;
   void allocDescriptorSets(
         const VkDevice& logicalDevice,
         const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
         std::vector<VkDescriptorSet>& descriptorSets
   );
   void destroyDescriptorPool(const VkDevice& logicalDevice);


private:

   VkDescriptorPool  m_descriptorPool;
};
