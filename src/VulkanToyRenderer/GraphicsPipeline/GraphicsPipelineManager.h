#pragma once

#include <vulkan/vulkan.h>

class GraphicsPipelineManager
{

public:

   GraphicsPipelineManager();
   ~GraphicsPipelineManager();
   createGraphicsPipeline();
   void destoryGraphicsPipeline(pipeline);
   void destoryPipelineLayout(pipeline);

private:

   void createShaderModules(
      VkShaderModule& vertexShaderModule,
      VkShaderModule& fragmentShaderModule
      logicalDevice
   );
   void createShaderStagesInfos(
      const VkShaderModule& vertexShaderModule,
      const VkShaderModule& fragmentShaderModule,
      VkPipelineShader& shaderStages
   );

   void createDynamicStates(VkPipelineDynamicStateCreateInfo& dynamicState);
   void createVertexShaderInputInfo(
         VkPipelineVertexInputStateCreateInfo& vertexInputInfo
   );
   void createInputAssemblyInfo(
      VkPipelineInputAssemblyStateCreateInfo& inputAssembly
   );

   void createViewport(VkViewport& viewport);
   void createScissor(VkRect2D& scissor);
   void createViewportStateInfo(
         VkPipelineViewportStateCreateInfo& viewportState
   );
   void createRasterizerInfo(
      VkPipelineRasterizationStateCreateInfo& rasterizer
   );
   void createMultisamplingInfo(
      VkPipelineMultisampleStateCreateInfo& multisampling
   );
   void createColorBlendingGlobalInfo(
      VkPipelineColorBlendStateCreateInfo& colorBlending
   );
   void createPipelineLayout();

   VkPipeline m_graphicsPipeline;
   VkPipelineLayour m_pipelineLayout;

};
