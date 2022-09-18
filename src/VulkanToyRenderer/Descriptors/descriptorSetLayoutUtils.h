#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace descriptorSetLayoutUtils
{

   void createDescriptorSetLayout(
         const VkDevice& logicalDevice,
         const std::vector<VkDescriptorType>& descriptorTypes,
         const std::vector<uint32_t>& descriptorBindings,
         const std::vector<VkShaderStageFlagBits>& descriptorStages,
         VkDescriptorSetLayout& descriptorSetLayout
   );
   void createDescriptorBindingLayout(
         const uint32_t bindingNumber,
         const VkDescriptorType& type,
         const VkShaderStageFlags& stageFlags,
         const std::vector<VkSampler>& immutableSamplers,
         VkDescriptorSetLayoutBinding& layout
   );

   void destroyDescriptorSetLayout(
         const VkDevice& logicalDevice,
         VkDescriptorSetLayout& descriptorSetLayout
   );

};
