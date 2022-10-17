#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Descriptors/DescriptorInfo.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Textures/Texture.h>

class DescriptorSets
{

public:

   void createDescriptorSets(
         const VkDevice logicalDevice,
         const std::vector<DescriptorInfo>& bindingUBOs,
         const std::vector<DescriptorInfo>& bindingSamplers,
         const std::vector<Texture>& texture,
         std::vector<VkBuffer>& uniformBuffers,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorPool& descriptorPool
   );

   const VkDescriptorSet& get(const uint32_t index) const;

private:

   template<typename T>
   void createDescriptorWriteInfo(
         const T& descriptorInfo,
         const VkDescriptorSet& descriptorSet,
         const uint32_t& dstBinding,
         const uint32_t& dstArrayElement,
         const VkDescriptorType& type,
         VkWriteDescriptorSet& descriptorWrite
   );


   std::vector<VkDescriptorSet> m_descriptorSets;
   std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;

};
