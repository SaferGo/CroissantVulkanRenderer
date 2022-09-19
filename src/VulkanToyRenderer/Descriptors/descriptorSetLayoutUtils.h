#pragma once

#include <vector>

#include <vulkan/vulkan.h>

struct DescriptorInfo
{
   VkDescriptorType        type;
   uint32_t                binding;
   VkShaderStageFlagBits   shaderStage;
};

namespace descriptorSetLayoutUtils
{

   void createDescriptorSetLayout(
         const VkDevice& logicalDevice,
         const std::vector<DescriptorInfo>& descriptorsInfo,
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
