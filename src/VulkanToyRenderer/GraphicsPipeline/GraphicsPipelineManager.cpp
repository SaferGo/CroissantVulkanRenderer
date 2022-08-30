#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipelineManager.cpp>

#include <vulkan/vulkan.h>

GraphicsPipelineManager::GraphicsPipelineManager() {}

GraphicsPipelineManager::~GraphicsPipelineManager() {}

// Improve this for more types(it can contain the type in the 
// param. with a vector containing the ShadrModules to create)
void GraphicsPipelineManager::createShaderModules(
      VkShaderModule& vertexShaderModule,
      VkShaderModule& fragmentShaderModule
      logicalDevice
{
   const std::vector<char> vertexShaderCode = (
         shaderManager::getBinaryDataFromFile("vert")
   );
   const std::vector<char> fragmentShaderCode = (
         shaderManager::getBinaryDataFromFile("frag")
   );

   vertexShaderModule = shaderManager::createShaderModule(
         vertexShaderCode,
         m_device.logicalDevice
   );
   fragmentShaderModule = shaderManager::createShaderModule(
         fragmentShaderCode,
         m_device.logicalDevice
   );

}

// Creates the info strucutres to link the shaders to specific pipeline stages.
void GraphicsPipelineManager::createShaderStagesInfos(
      const VkShaderModule& vertexShaderModule,
      const VkShaderModule& fragmentShaderModule,
      VkPipelineShader& shaderStages
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
   VkPipelineShaderStageCreateInfo fragmentShaderInfo{};
   fragmentShaderStageInfo.sType = (
         VK_STRCUTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
   );
   fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
   fragmentShaderStageInfo.module = fragmentShaderModule;
   fragmentShaderStageInfo.pName = "main";

   shadersStages [] = {
      vertexShaderStageInfo,
      fragmentShaderStageInfo
   };
}

void GraphicsPipelineManager::createDynamicStates(
      VkPipelineDynamicStateCreateInfo& dynamicState
) {
   // Here we define which states we want to modify dynamically during
   // runtime.
   std::vector<VkDynamicState> dynamicStates [] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
   };

   dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   dynamicState.dynameicStateCount = static_cast<uint32_t>(
         dynamicStates.size()
   );
   dynamicState.pDynamicStates = dynamicStates.data();
}

// Describes the format of the vertex data that will be passed to the
// vertex shader.
void GraphicsPipelineManager::createVertexShaderInputInfo(
   VkPipelineVertexInputStateCreateInfo& vertexInputInfo
) {
   vertexInputInfo.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
   );
   // Bindings: SPACING between data and whether the data is per-vertex or
   //           per-instance.
   vertexInputInfo.vertexBindingDescriptionCount = 0;
   // Attribute descriptions: Type of the attributes passsed to the vertex
   //                         shader, which binding to load them from and at
   //                         which OFFSET.
   vertexInputInfo.vertexAttributeDescriptionCount = 0;

}

// Describes the GEOMETRY PRIMITIVE and if the primitive restart should be
   // enabled.
   // (#) Primitive restart: Discards the most recent index values if those
   // elements formed an incomplete primitive, and restarts the primitive
   // assembly using the subsequent indices, but only assembling the immediatly
   // following element through the end of the originally specified elements.
void GraphicsPipelineManager::createInputAssemblyInfo(
      VkPipelineInputAssemblyStateCreateInfo& inputAssembly
) {
   inputAssembly.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
   );
   inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   inputAssembly.primitiveRestartEnable = VK_FALSE;
}

// Describes the region of the framebuffer that the output
// will be renderer to. This is almost always (0,0) to (width, height).
void GraphicsPipelineManager::createViewport(VkViewport& viewport)
{
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   viewport.width = (float) m_swapchainM.m_extent.width;
   viewport.height = (float) m_swapchainM.m_extent.height;
   // Depth values(these are the standard values)
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
}

void GraphicsPipelineManager::createScissor(VkRect2D& scissor)
{
   // In this case we want to draw the entire framebuffer
   scissor.offset = {0, 0};
   scissor.extent = m_swapchainM.m_extent;
}

// In the case that we turned viewport and scissor as dinamically, we need to
// specify their count at pipeline creation time.
void GraphicsPipelineManager::createViewportStateInfo(
         VkPipelineViewportStateCreateInfo& viewportState
) {
   viewportState.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
   );
   viewportState.viewportCount = 1;
   viewportState.scissorCount = 1;
}

void GraphicsPipelineManager::createRasterizerInfo(
      VkPipelineRasterizationStateCreateInfo& rasterizer
) {
   rasterizer.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
   );
   // If depthClampEnable is set to true, then fragments that are beyond the
   // near and far planes are clamped to them as opposed to discarding them.
   // This is useful in some special cases like SHADOW MAPS.
   rasterizer.depthClampEnable = VK_FALSE;
   // If rasterizerDiscardEnable is seto to true, then geometry never passes
   // through the rasterizer stage. This basically diables any output to the
   // framebuffer.
   rasterizer.rasterizerDiscardEnable = VK_FALSE;
   // Determines how fragments are generated for geometry:
   //    -VK_POLYGON_MODE_FILL: fills the area of the polygon with fragments.
   //    -VK_POLYGON_MODE_LINE: polygon edges are drawn as lines.
   //    -VK_POLYGON_MODE_POINT: polygon vertices are drawn as points.
   rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
   rasterizer.lineWidth = 1.0f;
   // Determines the type of face culling to use.
   rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
   rasterizer.frontFace = VK_FRONT_FACE_COUNTERCLOCKWISE;

   // Alters the depth values by adding a constant value or biasing them based
   // on a fragment's slope. Used sometimes for shadow mapping. We won't be
   // using it yet.
   rasterizer.depthBiasEnable = VK_FALSE;
   // Optional
   rasterizer.depthBiasConstantFactor = 0.0f;
   // Optional
   rasterizer.depthBiasClamp = 0.0f;
   // Optional
   rasterizer.depthBiasSlopeFactor = 0.0f;
}

void GraphicsPipelineManager::createMultisamplingInfo(
      VkPipelineMultisampleStateCreateInfo& multisampling
) {
   multisampling.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO
   );
   multisampling.sampleShadingEnable = VK_FALSE;
   multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
   // Optional
   multisampling.minSampleShading = 1.0f;
   // Optional
   multisampling.pSampleMask = nullptr;
   // Optional
   multisampling.alphaToCoverageEnable = VK_FALSE;
   // Optional
   multisampling.alphaToOneEnable = VK_FALSE;
}

void GraphicsPipelineManager::createColorBlendingGlobalInfo(
      VkPipelineColorBlendStateCreateInfo& colorBlending
) {
   // Color blending(attachment)
   
   // Contains the the configuration per attached framebuffer.
   // (For now, we won't use both config.)
   VkPipelineColorBlendAttachmentState colorBlendAttachment{};

   colorBlendAttachment.colorWriteMask = (
         VK_COLOR_COMPONENT_R_BIT |
         VK_COLOR_COMPONENT_G_BIT |
         VK_COLOR_COMPONENT_B_BIT |
         VK_COLOR_COMPONENT_A_BIT
   );
   colorBlendAttachment.blendEnable = VK_FALSE;
   colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
   colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
   colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
   colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
   colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
   colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
   
   // Color blending(global)
   colorBlending.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
   );
   colorBlending.logicOpEnable = VK_FALSE;
   colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
   colorBlending.attachmentCount = 1;
   colorBlending.pAttachments = &colorBlendAttachment;
   colorBlending.blendConstants[0] = 0.0f; // Optional
   colorBlending.blendConstants[1] = 0.0f; // Optional
   colorBlending.blendConstants[2] = 0.0f; // Optional
   colorBlending.blendConstants[3] = 0.0f; // Optional
}

// Interface that creates and allows us to communicate with the uniform
// values and push constants in the shaders.
void GraphicsPipelineManager::createPipelineLayout(
         logicalDevice,
         m_pipeline
) {
   pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   // Optional
   pipelineLayoutInfo.setLayoutCount= 0;
   // Optional
   pipelineLayoutInfo.pSetLayouts = nullptr;
   // Optional
   pipelineLayoutInfo.pushConstantRangeCount = 0;
   // Optional
   pipelineLayoutInfo.pPushContantRanges = nullptr;

   auto status = vkCreatePipelineLayout(
         m_logicalDevice,
         &pipelineLayoutInfo,
         nullptr,
         &m_pipelineLayout
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create pipeline layout!");
}

void GraphicsPipelineManager::createGraphicsPipeline()
{
   // -------------------Shader Modules--------------------
   VkShaderModule vertexShaderModule;
   VkShaderModule fragmentShaderModule;
   createShaderModules(vertexShaderModule, fragmentShaderModule);

   // -------------------Shader Stages--------------------
   VkPipelineShader shaderStages;
   createShaderStagesInfos(
         vertexShaderModule,
         fragmentShaderModule,
         shaderStages
   );

   // -------------------Fixed Functions------------------

   // Dynamic states
   VkPipelineDynamicStateCreateInfo dynamicState{};
   createDynamicStatesInfo(dynamicState);
   
   // Viewport
   VkViewport viewport{};
   createViewport(viewport);

   // Scissor
   VkRect2D scissor{};
   createScissor(scissor);

   // Viewport state info
   VkPipelineViewportStateCreateInfo viewportState{};
   createViewportStateInfo(viewportState);

   // Vertex input
   VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
   createVertexShaderInputInfo(vertexInputInfo);

   // Input assembly
   VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
   createInputAssemblyInfo(inputAssembly);

   // Rasterizer
   VkPipelineRasterizationStateCreateInfo rasterizer{};
   createRasterizerInfo(rasterizer);
   
   // Multisampling (disabled for now..)
   VkPipelineMultisampleStateCreateInfo multisampling{};
   createMultisamplingInfo(multisampling);

   // Color blending(global)
   VkPipelineColorBlendStateCreateInfo colorBlending{};
   createColorBlendingGlobalInfo(colorBlending);
   
   // Pipeline layout
   createPipelineLayout();

   // --------------Graphics pipeline creation------------

   VkGraphicsPipelineCreateInfo pipelineInfo{};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount = 2;
   pipelineInfo.pStages = shaderStages;
   // Reference to all the structures describing the fixed-function stage
   pipelineInfo.pVertexInputState = &vertexInputInfo;
   pipelineInfo.pInputAssemblyState = &inputAssembly;
   pipelineInfo.pViewportState = &viewportState;
   pipelineInfo.pRasterizationState = &rasterizer;
   pipelineInfo.pMultisampleState = &multisampling;
   pipelineInfo.pDepthStencilState = nullptr;
   pipelineInfo.pColorBlendState = &colorBlending;
   pipelineInfo.pDynamicState = &dynamicState;
   // Pipeline Layout
   pipelineInfo.layout = pipelineLayout;
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
         m_device.logicalDevice,
         VK_NULL_HANDLE,
         1,
         &pipelineInfo,
         nullptr
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create graphics pipeline!");

   // Since after the creation of the Graphics Pipeline the shaders are
   // already linked, we won't need the shader modules anymore.
   shaderManager::destroyShaderModule(
         vertexShaderModule,
         m_device.logicalDevice
   );
   shaderManager::destroyShaderModule(
         fragmentShaderModule,
         m_device.logicalDevice
   );
}

void GraphicsPipelineManager::destroyGraphicsPipeline(pipeline)
{
   vkDestroyPipeline(m_device.logicalDevice, m_graphicsPipeline, nullptr);
}

void GraphicsPipelineManager::destroyPipelineLayout(playout)
{
   vkDestroyPipelineLayout(m_device.logicalDevice, m_pipelineLayout, nullptr);
}
