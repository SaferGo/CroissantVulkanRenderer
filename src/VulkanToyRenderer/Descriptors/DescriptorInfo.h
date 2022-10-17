#pragma once

#include <vulkan/vulkan.h>

struct DescriptorInfo
{
   int bindingNumber;
   VkDescriptorType descriptorType;
   VkShaderStageFlagBits shaderStage;
};

