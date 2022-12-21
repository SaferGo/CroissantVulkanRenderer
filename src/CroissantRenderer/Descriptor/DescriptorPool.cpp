#include <CroissantRenderer/Descriptor/DescriptorPool.h>

#include <array>
#include <cstring>
#include <cmath>
#include <iostream>

#include <vulkan/vulkan.h>

DescriptorPool::DescriptorPool() {}

DescriptorPool::DescriptorPool(
      const VkDevice& logicalDevice,
      const std::vector<VkDescriptorPoolSize> poolSizes,
      const uint32_t descriptorSetsCount
) : m_logicalDevice(logicalDevice)
{

   if (poolSizes.size() == 0)
      throw std::runtime_error("Failed to create descriptor pool!");

   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
   poolInfo.pPoolSizes = poolSizes.data();
   // Specifies the maximum number of descriptor sets that may be allocated.
   poolInfo.maxSets = descriptorSetsCount;
   
   auto status = vkCreateDescriptorPool(
         logicalDevice,
         &poolInfo,
         nullptr,
         &m_descriptorPool
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor pool!");
}

DescriptorPool::~DescriptorPool() {}

const VkDescriptorPool& DescriptorPool::get() const
{
   return m_descriptorPool;
}

/*
 * Allocates all the descriptor set from all the objs.
 */
void DescriptorPool::allocDescriptorSets(
      const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
      std::vector<VkDescriptorSet>& descriptorSets
) {
   VkDescriptorSetAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = m_descriptorPool;
   allocInfo.descriptorSetCount = static_cast<uint32_t>(
         descriptorSets.size()
   );
   allocInfo.pSetLayouts = descriptorSetLayouts.data();


   auto status = vkAllocateDescriptorSets(
         m_logicalDevice,
         &allocInfo,
         descriptorSets.data()
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate descriptr sets!");
}


void DescriptorPool::destroy()
{
   vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);
}
