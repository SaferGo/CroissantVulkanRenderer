#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Pipeline/Pipeline.h>
#include <CroissantRenderer/Descriptor/DescriptorInfo.h>

class Compute : public Pipeline
{

public:

   Compute();
   Compute(
         const VkDevice& logicalDevice,
         const ShaderInfo& shaderInfo,
         const std::vector<DescriptorInfo>& bufferInfos,
         const std::vector<VkPushConstantRange>& pushConstantRanges
   );
   ~Compute();

private:

   void createShaderStageInfo(
         const VkShaderModule& shaderModule,
         const shaderType& type,
         VkPipelineShaderStageCreateInfo& shaderStageInfo
   ) override;
   void createDescriptorSetLayout(
         const std::vector<DescriptorInfo>& bufferInfos
   );
};

