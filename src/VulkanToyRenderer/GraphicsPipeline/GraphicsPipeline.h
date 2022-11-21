#pragma once

#include <vector>
#include <array>
#include <string>

#include <vulkan/vulkan.h>

enum class GraphicsPipelineType
{
   PBR = 0,
   LIGHT = 1,
   SKYBOX = 2,
   SHADOWMAP = 3
};

enum shaderType
{
   NONE = 0,
   VERTEX = 1,
   FRAGMENT = 2
};

struct ShaderInfo
{
   shaderType type;
   std::string fileName;
};

class GraphicsPipeline
{

public:

   GraphicsPipeline();
   ~GraphicsPipeline();
   GraphicsPipeline(
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
   const VkPipeline& get() const;
   const VkPipelineLayout& getPipelineLayout() const;
   const GraphicsPipelineType getType() const;
   const std::vector<size_t>& getModelIndices() const;
   void destroy();

private:

   void createShaderModule(
      const ShaderInfo& shaderInfos,
      VkShaderModule& shaderModule
   );
   void createShaderStageInfo(
      const VkShaderModule& shaderModule,
      const shaderType& type,
      VkPipelineShaderStageCreateInfo& shaderStageInfo
   );
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
   void createPipelineLayout(
         const VkDescriptorSetLayout& descriptorSetLayout
   );

   VkDevice             m_logicalDevice;
   GraphicsPipelineType m_type;

   VkPipeline           m_graphicsPipeline;
   VkPipelineLayout     m_pipelineLayout;

   // Observer pointer
   std::vector<size_t>* m_opModelIndices;
};
