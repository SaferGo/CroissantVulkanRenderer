#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipeline.h>

#include <vector>
#include <array>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/ShaderManager/shaderManager.h>
#include <VulkanToyRenderer/GraphicsPipeline/renderTargetUtils.h>

GraphicsPipeline::GraphicsPipeline() {}

GraphicsPipeline::~GraphicsPipeline() {}

GraphicsPipeline::GraphicsPipeline(
      const VkDevice& logicalDevice,
      const GraphicsPipelineType type,
      const VkExtent2D& extent,
      const VkRenderPass& renderPass,
      const VkDescriptorSetLayout& descriptorSetLayout,
      const std::string& vertexShaderFileName,
      const std::string& fragmentShaderFileName,
      const VkSampleCountFlagBits& samplesCount,
      VkVertexInputBindingDescription vertexBindingDescriptions,
      std::vector<VkVertexInputAttributeDescription> vertexAttribDescriptions,
      std::vector<size_t>* modelIndices
) : m_type(type), m_opModelIndices(modelIndices) {
   // -------------------Shader Modules--------------------

   VkShaderModule vertexShaderModule;
   VkShaderModule fragmentShaderModule;
   createShaderModules(
         logicalDevice,
         vertexShaderFileName,
         vertexShaderModule,
         fragmentShaderFileName,
         fragmentShaderModule
   );

   // -------------------Shader Stages--------------------

   VkPipelineShaderStageCreateInfo shaderStagesInfos[2];
   createShaderStagesInfos(
         vertexShaderModule,
         fragmentShaderModule,
         shaderStagesInfos
   );

   // -------------------Fixed Functions------------------

   // Here we define which states we want to modify dynamically during
   // runtime.
   std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
   };
   // Dynamic states
   VkPipelineDynamicStateCreateInfo dynamicStatesInfo{};
   createDynamicStatesInfo(dynamicStates, dynamicStatesInfo);
   
   // Viewport
   VkViewport viewport{};
   createViewport(viewport, extent);

   // Scissor
   VkRect2D scissor{};
   createScissor(scissor, extent);

   // Viewport state info
   VkPipelineViewportStateCreateInfo viewportStateInfo{};
   createViewportStateInfo(viewportStateInfo);

   // -Vertex input(attributes)

   // Gets the binding and descriptions of the triangle's vertices and
   // vertex attributes(one array containing both).
   VkVertexInputBindingDescription bindingDescription = (
         vertexBindingDescriptions
   );
   std::vector<VkVertexInputAttributeDescription> attribDescriptions = (
         vertexAttribDescriptions
   );
   VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
   createVertexShaderInputInfo(
         bindingDescription,
         attribDescriptions,
         vertexInputInfo
   );

   // Input assembly
   VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
   createInputAssemblyInfo(inputAssemblyInfo);

   // Rasterizer
   VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
   createRasterizerInfo(rasterizerInfo);
   
   // Multisampling (disabled for now..)
   VkPipelineMultisampleStateCreateInfo multisamplingInfo{};
   createMultisamplingInfo(samplesCount, multisamplingInfo);

   // Color blending(attachment)
   VkPipelineColorBlendAttachmentState colorBlendAttachment{};
   createColorBlendingAttachment(colorBlendAttachment);

   // Color blending(global)
   VkPipelineColorBlendStateCreateInfo colorBlendingInfo{};
   createColorBlendingGlobalInfo(colorBlendAttachment, colorBlendingInfo);
   
   // Pipeline layout
   createPipelineLayout(logicalDevice, descriptorSetLayout);

   // Depth and stencil
   VkPipelineDepthStencilStateCreateInfo depthStencil{};
   renderTargetUtils::depthBuffer::createDepthStencilStateInfo(
         m_type,
         depthStencil
   );
   
   // --------------Graphics pipeline creation------------

   VkGraphicsPipelineCreateInfo pipelineInfo{};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount = 2;
   pipelineInfo.pStages = shaderStagesInfos;
   // Reference to all the structures describing the fixed-function stage
   pipelineInfo.pVertexInputState = &vertexInputInfo;
   pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
   pipelineInfo.pViewportState = &viewportStateInfo;
   pipelineInfo.pRasterizationState = &rasterizerInfo;
   pipelineInfo.pMultisampleState = &multisamplingInfo;
   pipelineInfo.pDepthStencilState = &depthStencil;
   pipelineInfo.pColorBlendState = &colorBlendingInfo;
   pipelineInfo.pDynamicState = &dynamicStatesInfo;
   // Pipeline Layout
   pipelineInfo.layout = m_pipelineLayout;
   // Render pass and the index of the sub pass where this graphics
   // pipeline will be used.
   pipelineInfo.renderPass = renderPass;
   pipelineInfo.subpass = 0;
   // Pipelines derivatives(less expensive to set up pipelines when they
   // have much functionality in common whith an existing pupeline and
   // switching between pipelines from the same parent can also be done
   // quicker)
   // Optional
   pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
   // Optional
   pipelineInfo.basePipelineIndex = -1;
   
   auto status = vkCreateGraphicsPipelines(
         logicalDevice,
         VK_NULL_HANDLE,
         1,
         &pipelineInfo, nullptr,
         &m_graphicsPipeline
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create graphics pipeline!");

   // Since after the creation of the Graphics Pipeline the shaders are
   // already linked, we won't need the shader modules anymore.
   shaderManager::destroyShaderModule(
         vertexShaderModule,
         logicalDevice
   );
   shaderManager::destroyShaderModule(
         fragmentShaderModule,
         logicalDevice
   );
}

void GraphicsPipeline::createShaderModules(
      const VkDevice& logicalDevice,
      const std::string& vertexShaderFileName,
      VkShaderModule& vertexShaderModule,
      const std::string& fragmentShaderFileName,
      VkShaderModule& fragmentShaderModule
) {
   const std::vector<char> vertexShaderCode = (
         shaderManager::getBinaryDataFromFile("vert-" + vertexShaderFileName)
   );
   const std::vector<char> fragmentShaderCode = (
         shaderManager::getBinaryDataFromFile("frag-" + fragmentShaderFileName)
   );

   vertexShaderModule = shaderManager::createShaderModule(
         vertexShaderCode,
         logicalDevice
   );
   fragmentShaderModule = shaderManager::createShaderModule(
         fragmentShaderCode,
         logicalDevice
   );

}

/*
 * Creates the info strucutres to link the shaders to specific pipeline stages.
 */
void GraphicsPipeline::createShaderStagesInfos(
      const VkShaderModule& vertexShaderModule,
      const VkShaderModule& fragmentShaderModule,
      VkPipelineShaderStageCreateInfo (&shaderStagesInfos)[2]
) {
   // - Vertex Shader
   VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
   vertexShaderStageInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
   );
   vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
   vertexShaderStageInfo.module = vertexShaderModule;
   // Entry point: Name of the function to invoke.
   vertexShaderStageInfo.pName = "main";
   // pSpecializationInfo -> Specifies values for shader constants. This
   // optimizes the shaders avoiding the use of if-statements.
   // ..

   // - Fragment Shader
   VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
   fragmentShaderStageInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
   );
   fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
   fragmentShaderStageInfo.module = fragmentShaderModule;
   fragmentShaderStageInfo.pName = "main";

   shaderStagesInfos[0] = vertexShaderStageInfo;
   shaderStagesInfos[1] = fragmentShaderStageInfo;
}

void GraphicsPipeline::createDynamicStatesInfo(
      const std::vector<VkDynamicState>& dynamicStates,
      VkPipelineDynamicStateCreateInfo& dynamicStatesInfo
) {
   dynamicStatesInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
   );
   dynamicStatesInfo.dynamicStateCount = static_cast<uint32_t>(
         dynamicStates.size()
   );
   dynamicStatesInfo.pDynamicStates = dynamicStates.data();
}

/*
 * Describes the format of the vertex data that will be passed to the
 * vertex shader.
 */
void GraphicsPipeline::createVertexShaderInputInfo(
      const VkVertexInputBindingDescription& bindingDescription,
      const std::vector<VkVertexInputAttributeDescription>&
         attribDescriptions,
      VkPipelineVertexInputStateCreateInfo& vertexInputInfo
) {
   vertexInputInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
   );
   // Bindings: Number of vertex bindings descriptions provided in
   //           pVertexBindingDescriptions.
   vertexInputInfo.vertexBindingDescriptionCount = 1;
   // Attribute descriptions: Type of the attributes passsed to the vertex
   //                         shader, which binding to load them from and at
   //                         which OFFSET.
   vertexInputInfo.vertexAttributeDescriptionCount = (
         static_cast<uint32_t>(attribDescriptions.size())
   );
   vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
   vertexInputInfo.pVertexAttributeDescriptions = attribDescriptions.data();
}

// Describes the GEOMETRY PRIMITIVE and if the primitive restart should be
   // enabled.
   // (#) Primitive restart: Discards the most recent index values if those
   // elements formed an incomplete primitive, and restarts the primitive
   // assembly using the subsequent indices, but only assembling the immediatly
   // following element through the end of the originally specified elements.
void GraphicsPipeline::createInputAssemblyInfo(
      VkPipelineInputAssemblyStateCreateInfo& inputAssemblyInfo
) {
   inputAssemblyInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
   );
   inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
}

// Describes the region of the framebuffer that the output
// will be renderer to. This is almost always (0,0) to (width, height).
void GraphicsPipeline::createViewport(
      VkViewport& viewport,
      const VkExtent2D& extent
) {
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   viewport.width = (float) extent.width;
   viewport.height = (float) extent.height;
   // Depth values(these are the standard values)
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
}

void GraphicsPipeline::createScissor(
      VkRect2D& scissor,
      const VkExtent2D& extent
) {
   // In this case we want to draw the entire framebuffer
   scissor.offset = {0, 0};
   scissor.extent = extent;
}

// In the case that we turned viewport and scissor as dinamically, we need to
// specify their count at pipeline creation time.
void GraphicsPipeline::createViewportStateInfo(
         VkPipelineViewportStateCreateInfo& viewportStateInfo
) {
   viewportStateInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
   );
   viewportStateInfo.viewportCount = 1;
   viewportStateInfo.scissorCount = 1;
}

void GraphicsPipeline::createRasterizerInfo(
      VkPipelineRasterizationStateCreateInfo& rasterizerInfo
) {
   rasterizerInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
   );
   // If depthClampEnable is set to true, then fragments that are beyond the
   // near and far planes are clamped to them as opposed to discarding them.
   // This is useful in some special cases like SHADOW MAPS.
   rasterizerInfo.depthClampEnable = VK_FALSE;
   // If rasterizerDiscardEnable is seto to true, then geometry never passes
   // through the rasterizer stage. This basically diables any output to the
   // framebuffer.
   rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
   // Determines how fragments are generated for geometry:
   //    -VK_POLYGON_MODE_FILL: fills the area of the polygon with fragments.
   //    -VK_POLYGON_MODE_LINE: polygon edges are drawn as lines.
   //    -VK_POLYGON_MODE_POINT: polygon vertices are drawn as points.
   rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
   rasterizerInfo.lineWidth = 1.0f;

   // Determines the type of face culling to use.
   if (m_type == GraphicsPipelineType::SKYBOX)
   {
      rasterizerInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
   }
   else
      rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;

   rasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

   // Alters the depth values by adding a constant value or biasing them based
   // on a fragment's slope. Used sometimes for shadow mapping. We won't be
   // using it yet.
   rasterizerInfo.depthBiasEnable = VK_FALSE;
   // Optional
   rasterizerInfo.depthBiasConstantFactor = 0.0f;
   // Optional
   rasterizerInfo.depthBiasClamp = 0.0f;
   // Optional
   rasterizerInfo.depthBiasSlopeFactor = 0.0f;
}

void GraphicsPipeline::createMultisamplingInfo(
      const VkSampleCountFlagBits& samplesCount,
      VkPipelineMultisampleStateCreateInfo& multisamplingInfo
) {
   multisamplingInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
   );
   multisamplingInfo.sampleShadingEnable = VK_TRUE;
   multisamplingInfo.rasterizationSamples = samplesCount;
   // Optional
   multisamplingInfo.minSampleShading = 1.0f;
   // Optional
   multisamplingInfo.pSampleMask = nullptr;
   // Optional
   multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
   // Optional
   multisamplingInfo.alphaToOneEnable = VK_FALSE;
}

// Contains the the configuration per attached framebuffer.
// (For now, we won't use both config.)
void GraphicsPipeline::createColorBlendingAttachment(
      VkPipelineColorBlendAttachmentState& colorBlendAttachment
) {
   colorBlendAttachment.colorWriteMask = (
         VK_COLOR_COMPONENT_R_BIT |
         VK_COLOR_COMPONENT_G_BIT |
         VK_COLOR_COMPONENT_B_BIT |
         VK_COLOR_COMPONENT_A_BIT
   );
   colorBlendAttachment.blendEnable = VK_FALSE;
}


void GraphicsPipeline::createColorBlendingGlobalInfo(
      const VkPipelineColorBlendAttachmentState& colorBlendAttachment,
      VkPipelineColorBlendStateCreateInfo& colorBlendingInfo
) {
   colorBlendingInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
   );
   colorBlendingInfo.logicOpEnable = VK_FALSE;
   colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
   colorBlendingInfo.attachmentCount = 1;
   colorBlendingInfo.pAttachments = &colorBlendAttachment;
   colorBlendingInfo.blendConstants[0] = 0.0f; // Optional
   colorBlendingInfo.blendConstants[1] = 0.0f; // Optional
   colorBlendingInfo.blendConstants[2] = 0.0f; // Optional
   colorBlendingInfo.blendConstants[3] = 0.0f; // Optional
}

// Interface that creates and allows us to communicate with the uniform
// values and push constants in the shaders.
void GraphicsPipeline::createPipelineLayout(
   const VkDevice& logicalDevice,
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
         logicalDevice,
         &pipelineLayoutInfo,
         nullptr,
         &m_pipelineLayout
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create pipeline layout!");
}


const VkPipeline& GraphicsPipeline::get() const
{
   return m_graphicsPipeline;
}

// TODO: make this safer.
const std::vector<size_t>& GraphicsPipeline::getModelIndices() const
{
   return *m_opModelIndices;
}

const VkPipelineLayout& GraphicsPipeline::getPipelineLayout() const
{
   return m_pipelineLayout;
}

void GraphicsPipeline::destroy(const VkDevice& logicalDevice)
{
   vkDestroyPipeline(logicalDevice, m_graphicsPipeline, nullptr);
   vkDestroyPipelineLayout(logicalDevice, m_pipelineLayout, nullptr);
}

const GraphicsPipelineType GraphicsPipeline::getType() const
{
   return m_type;
}
