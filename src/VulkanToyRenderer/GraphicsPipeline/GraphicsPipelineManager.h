#pragma once

#include <vector>
#include <array>

#include <vulkan/vulkan.h>

class GraphicsPipelineManager
{

public:

   GraphicsPipelineManager();
   ~GraphicsPipelineManager();
   void createGraphicsPipeline(
         const VkDevice& logicalDevice,
         const VkExtent2D& extent,
         const VkRenderPass& renderPass,
         const VkDescriptorSetLayout& descriptorSetLayout
   );
   const VkPipeline& getGraphicsPipeline() const;

   VkPipelineLayout& getPipelineLayout();
   void destroyGraphicsPipeline(const VkDevice& logicalDevice);
   void destroyPipelineLayout(const VkDevice& logicalDevice);

private:

   void createShaderModules(
      VkShaderModule& vertexShaderModule,
      VkShaderModule& fragmentShaderModule,
      const VkDevice& logicalDevice
   );
   void createShaderStagesInfos(
      const VkShaderModule& vertexShaderModule,
      const VkShaderModule& fragmentShaderModule,
      VkPipelineShaderStageCreateInfo (&shaderStagesInfos)[2]
   );

   void createDynamicStatesInfo(
      const std::vector<VkDynamicState>& dynamicStates,
      VkPipelineDynamicStateCreateInfo& dynamicStatesInfo
   );
   void createVertexShaderInputInfo(
         const VkVertexInputBindingDescription& bindingDescription,
         const std::array<VkVertexInputAttributeDescription, 3>&
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
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout
   );


   VkPipeline m_graphicsPipeline;
   VkPipelineLayout m_pipelineLayout;
};
