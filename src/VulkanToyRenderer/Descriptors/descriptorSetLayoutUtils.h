#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Descriptors/DescriptorInfo.h>

namespace descriptorSetLayoutUtils
{

   void createDescriptorSetLayout(
         const VkDevice& logicalDevice,
         const std::vector<DescriptorInfo>& uboInfo,
         const std::vector<DescriptorInfo>& samplersInfo,
         VkDescriptorSetLayout& descriptorSetLayout
   );
   void createDescriptorBindingLayout(
         const DescriptorInfo& descriptorInfo,
         const std::vector<VkSampler>& immutableSamplers,
         VkDescriptorSetLayoutBinding& layout
   );

   void destroyDescriptorSetLayout(
         const VkDevice& logicalDevice,
         VkDescriptorSetLayout& descriptorSetLayout
   );

};
