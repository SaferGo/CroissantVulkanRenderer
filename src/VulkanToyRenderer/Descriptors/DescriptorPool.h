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
   const VkDescriptorPool& get() const;
   void allocDescriptorSets(
         const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
         std::vector<VkDescriptorSet>& descriptorSets
   );
   void destroy();


private:

   VkDevice          m_logicalDevice;

   VkDescriptorPool  m_descriptorPool;
};
