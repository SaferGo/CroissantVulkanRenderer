#pragma once

#include <vector>
#include <array>
#include <string>

#include <vulkan/vulkan.h>

class GraphicsPipeline
{

public:

   GraphicsPipeline();
   ~GraphicsPipeline();
   GraphicsPipeline(
         const VkDevice& logicalDevice,
         const VkExtent2D& extent,
         const VkRenderPass& renderPass,
         const VkDescriptorSetLayout& descriptorSetLayout,
         const std::string& vertexShaderFileName,
         const std::string& fragmentShaderFileName,
         std::vector<size_t>* modelIndices
   );
   const VkPipeline& get() const;
   const VkPipelineLayout& getPipelineLayout() const;
   const std::vector<size_t>& getModelIndices() const;
   void destroy(const VkDevice& logicalDevice);

private:

   void createShaderModules(
      const VkDevice& logicalDevice,
      const std::string& vertexShaderFileName,
      VkShaderModule& vertexShaderModule,
      const std::string& fragmentShaderFileName,
      VkShaderModule& fragmentShaderModule
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

   // Observer pointer
   std::vector<size_t>* m_opModelIndices;
};
