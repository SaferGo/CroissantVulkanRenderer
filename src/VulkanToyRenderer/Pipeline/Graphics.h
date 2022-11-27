#pragma once

#include <vector>
#include <array>
#include <string>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Pipeline/Pipeline.h>

enum class GraphicsPipelineType
{
   PBR = 0,
   LIGHT = 1,
   SKYBOX = 2,
   SHADOWMAP = 3
};

class Graphics : public Pipeline
{

public:

   Graphics();
   Graphics(
         const VkDevice& logicalDevice,
         const GraphicsPipelineType type,
         const VkExtent2D& extent,
         const VkRenderPass& renderPass,
         const VkDescriptorSetLayout& descriptorSetLayout,
         const std::vector<ShaderInfo>& shaderInfos,
         const VkSampleCountFlagBits& samplesCount,
         VkVertexInputBindingDescription vertexBindingDescriptions,
         std::vector<VkVertexInputAttributeDescription>
            vertexAttribDescriptions,
         std::vector<size_t>* modelIndices
   );
   ~Graphics();
   const GraphicsPipelineType getGraphicsPipelineType() const;
   const std::vector<size_t>& getModelIndices() const;

private:

   void createShaderStageInfo(
      const VkShaderModule& shaderModule,
      const shaderType& type,
      VkPipelineShaderStageCreateInfo& shaderStageInfo
   ) override;
   void createDynamicStatesInfo(
      const std::vector<VkDynamicState>& dynamicStates,
      VkPipelineDynamicStateCreateInfo& dynamicStatesInfo
   );
   void createVertexShaderInputInfo(
         const VkVertexInputBindingDescription& bindingDescription,
         const std::vector<VkVertexInputAttributeDescription>&
            attribDescriptions,
         VkPipelineVertexInputStateCreateInfo& vertexInputInfo
   );
   void createInputAssemblyInfo(
      VkPipelineInputAssemblyStateCreateInfo& inputAssemblyInfo
   );

   void createViewport(VkViewport& viewport, const VkExtent2D& extent);
   void createScissor(VkRect2D& scissor, const VkExtent2D& extent);
   void createViewportStateInfo(
         VkPipelineViewportStateCreateInfo& viewportStateInfo
   );
   void createRasterizerInfo(
      VkPipelineRasterizationStateCreateInfo& rasterizerInfo
   );
   void createMultisamplingInfo(
      const VkSampleCountFlagBits& samplesCount,
      VkPipelineMultisampleStateCreateInfo& multisamplingInfo
   );
   void createColorBlendingAttachment(
      VkPipelineColorBlendAttachmentState& colorBlendAttachment
   );
   void createColorBlendingGlobalInfo(
      const VkPipelineColorBlendAttachmentState& colorBlendAttachment,
      VkPipelineColorBlendStateCreateInfo& colorBlendingInfo
   );

   GraphicsPipelineType m_gType;

   // Observer pointer
   std::vector<size_t>* m_opModelIndices;
};
