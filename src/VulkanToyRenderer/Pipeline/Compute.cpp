#include <VulkanToyRenderer/Pipeline/Compute.h>

#include <vector>
#include <array>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/ShaderManager/shaderManager.h>

Compute::Compute() {}

Compute::~Compute() {}

Compute::Compute(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      const ShaderInfo& shaderInfo
) : Pipeline(logicalDevice, PipelineType::COMPUTE)
{

   VkShaderModule shaderModule;
   VkPipelineShaderStageCreateInfo shaderStageInfo;

   createShaderModule(
         shaderInfo,
         shaderModule
   );
   createShaderStageInfo(
         shaderModule,
         shaderInfo.type,
         shaderStageInfo
   );

   // -------------------Fixed Functions------------------

   // Pipeline layout
   createPipelineLayout(descriptorSetLayout);

   // --------------Compute pipeline creation------------

   VkComputePipelineCreateInfo pipelineInfo{};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
   pipelineInfo.stage = shaderStageInfo;
   pipelineInfo.layout = m_pipelineLayout;
   pipelineInfo.basePipelineHandle = 0;
   pipelineInfo.basePipelineIndex = 0;
   
   auto status = vkCreateComputePipelines(
         logicalDevice,
         VK_NULL_HANDLE,
         1,
         &pipelineInfo, nullptr,
         &m_pipeline
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create compute pipeline!");

   // Since after the creation of the Compute Pipeline the shaders are
   // already linked, we won't need the shader modules anymore.
   shaderManager::destroyShaderModule(
         shaderModule,
         logicalDevice
   );
}

/*
 * Creates the info strucutres to link the shaders to specific pipeline stages.
 */
void Compute::createShaderStageInfo(
      const VkShaderModule& shaderModule,
      const shaderType& type,
      VkPipelineShaderStageCreateInfo& shaderStageInfo
) {
   shaderStageInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
   );

   if (type == shaderType::COMPUTE)
      shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
   else
      throw std::runtime_error("Shader type doesn't exist");

   shaderStageInfo.module = shaderModule;
   // Entry point: Name of the function to invoke.
   shaderStageInfo.pName = "main";
   shaderStageInfo.pNext = nullptr;
   shaderStageInfo.flags = 0;
   shaderStageInfo.pSpecializationInfo = nullptr;
   
}
