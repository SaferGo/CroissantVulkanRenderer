#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>

#include <array>
#include <cstring>
#include <cmath>
#include <iostream>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Descriptors/DescriptorTypes/DescriptorTypes.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Settings/config.h>

DescriptorPool::DescriptorPool() {}
DescriptorPool::~DescriptorPool() {}

void DescriptorPool::createDescriptorPool(
      const VkDevice& logicalDevice,
      const std::vector<VkDescriptorType>& descriptorTypes,
      const std::vector<uint32_t> descriptorSizes,
      const uint32_t descriptorSetsCount
) {

   if (descriptorTypes.size() == 0 ||
       descriptorSizes.size() == 0 ||
       descriptorTypes.size() != descriptorSizes.size()
   ) {
      throw std::runtime_error("Failed to create descriptor pool!");
   }

   std::vector<VkDescriptorPoolSize>poolSizes(descriptorTypes.size());
   
   for (size_t i = 0; i < descriptorTypes.size(); i++)
      createPoolSize(descriptorSizes[i], descriptorTypes[i], poolSizes[i]);

   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
   poolInfo.pPoolSizes = poolSizes.data();
   // Specifies the maximum number of descriptor sets that may be allocated.
   poolInfo.maxSets = static_cast<uint32_t>(descriptorSetsCount);
   
   auto status = vkCreateDescriptorPool(
         logicalDevice,
         &poolInfo,
         nullptr,
         &m_descriptorPool
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor pool!");
}

void DescriptorPool::createPoolSize(
      const size_t size,
      const VkDescriptorType& type,
      VkDescriptorPoolSize& poolSize
) {
   poolSize.type = type;
   poolSize.descriptorCount = static_cast<uint32_t>(size);
}

/*
 * Allocates all the descriptor set from all the objs.
 */
void DescriptorPool::allocDescriptorSets(
      const VkDevice& logicalDevice,
      std::vector<VkDescriptorSet>& descriptorSets,
      std::vector<VkDescriptorSetLayout>& descriptorSetLayouts
) {
   VkDescriptorSetAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = m_descriptorPool;
   allocInfo.descriptorSetCount = static_cast<uint32_t>(
         descriptorSets.size()
   );
   allocInfo.pSetLayouts = descriptorSetLayouts.data();


   auto status = vkAllocateDescriptorSets(
         logicalDevice,
         &allocInfo,
         descriptorSets.data()
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate descriptr sets!");
}


void DescriptorPool::destroyDescriptorPool(const VkDevice& logicalDevice)
{
   vkDestroyDescriptorPool(logicalDevice, m_descriptorPool, nullptr);
}


