#include <VulkanToyRenderer/Pipeline/Pipeline.h>

#include <vector>
#include <string>
#include <stdexcept>

#include <VulkanToyRenderer/Shader/shaderManager.h>
#include <VulkanToyRenderer/Descriptor/descriptorSetLayoutManager.h>

Pipeline::Pipeline() {}

Pipeline::Pipeline(
      const VkDevice& logicalDevice,
      const PipelineType& type
) : m_logicalDevice(logicalDevice),
    m_type(type)
{}

Pipeline::~Pipeline() {}

void Pipeline::createShaderModule(
      const ShaderInfo& shaderInfo,
      VkShaderModule& shaderModule
) {
   std::vector<char> shaderCode;

   if (shaderInfo.type == shaderType::VERTEX)
   {
      shaderCode = (
            shaderManager::getBinaryDataFromFile("vert-" + shaderInfo.fileName)
      );
   } else if (shaderInfo.type == shaderType::FRAGMENT)
   {
      shaderCode = (
            shaderManager::getBinaryDataFromFile("frag-" + shaderInfo.fileName)
      );
   } else if (shaderInfo.type == shaderType::COMPUTE)
   {
      shaderCode = (
            shaderManager::getBinaryDataFromFile("comp-" + shaderInfo.fileName)
      );
   } else
   {
      throw std::runtime_error("Shader type doesn't exist.");
   }

   shaderModule = shaderManager::createShaderModule(
         shaderCode,
         m_logicalDevice
   );
}

/*
 * Interface that creates and allows us to communicate with the uniform
 * values and push constants in the shaders.
*/
void Pipeline::createPipelineLayout(
      const VkDescriptorSetLayout& descriptorSetLayout
) {
   VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
   pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   // In this case we gonna bind the descriptor layout.
   pipelineLayoutInfo.setLayoutCount= 1;
   pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
   // Optional
   pipelineLayoutInfo.pushConstantRangeCount = 0;
   // Optional
   pipelineLayoutInfo.pPushConstantRanges = nullptr;

   auto status = vkCreatePipelineLayout(
         m_logicalDevice,
         &pipelineLayoutInfo,
         nullptr,
         &m_pipelineLayout
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create pipeline layout!");
}


const VkPipeline& Pipeline::get() const
{
   return m_pipeline;
}

const VkPipelineLayout& Pipeline::getPipelineLayout() const
{
   return m_pipelineLayout;
}

void Pipeline::destroy()
{
   descriptorSetLayoutManager::destroyDescriptorSetLayout(
         m_logicalDevice,
         m_descriptorSetLayout
   );
   vkDestroyPipeline(m_logicalDevice, m_pipeline, nullptr);
   vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
}

const PipelineType& Pipeline::getType() const
{
   return m_type;
}

const VkDescriptorSetLayout& Pipeline::getDescriptorSetLayout() const
{
   return m_descriptorSetLayout;
}
