#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Pipeline/Pipeline.h>

class Compute : public Pipeline
{

public:

   Compute();
   Compute(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         const ShaderInfo& shaderInfo
   );
   ~Compute();

private:

   void createShaderStageInfo(
         const VkShaderModule& shaderModule,
         const shaderType& type,
         VkPipelineShaderStageCreateInfo& shaderStageInfo
   ) override;
};
