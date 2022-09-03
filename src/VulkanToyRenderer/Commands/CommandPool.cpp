#include <VulkanToyRenderer/Commands/CommandPool.h>

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>

CommandPool::CommandPool(
      const VkDevice& logicalDevice,
      QueueFamilyIndices& queueFamilyIndices
) {
   m_logicalDevice = logicalDevice;
   m_queueFamilyIndices = queueFamilyIndices;

   VkCommandPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   // Allows command buffers to be rerecorded individually, without this
   // flag they all have to reset together.
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   poolInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamily.value();

   auto status = vkCreateCommandPool(
         m_logicalDevice,
         &poolInfo,
         nullptr,
         &m_commandPool
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create command pool!");
}

void CommandPool::destroyCommandPool()
{
   vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
}

void CommandPool::allocCommandBuffer(
      const VkCommandBufferAllocateInfo& allocInfo
) {
   VkCommandBuffer newCommandBuffer;
   auto status = vkAllocateCommandBuffers(
         m_logicalDevice,
         &allocInfo,
         &newCommandBuffer
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate command buffers!");

   m_commandBuffers.push_back(std::move(newCommandBuffer));
}

const VkCommandBuffer& CommandPool::getCommandBuffer(const uint32_t index)
{
   return m_commandBuffers[index];
}

void CommandPool::recordCommandBuffer(
      const VkFramebuffer& framebuffer,
      const VkRenderPass& renderPass,
      const VkExtent2D& extent,
      const VkPipeline& graphicsPipeline,
      const uint32_t commandBufferIndex
) {
   // Specifies some details about the usage of this specific command
   // buffer.
   VkCommandBufferBeginInfo beginInfo{};
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   // Optional
   // Specifies how we're goint to use the command buffer:
   //    -VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer
   //    will be rerecorded right after executing it once.
   //    -VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary
   //    command buffer that will be entirely within a single render pass.
   //    -VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer
   //    can be resubmitted while it is also already pending execution.
   // For now we won't use any of them.
   beginInfo.flags = 0;
   // Optional
   // Relevant only for secondary command buffers. It specifies which state
   // to inherit from the calling primary command buffers.
   beginInfo.pInheritanceInfo = nullptr;
   
   // If the command buffer was already recorded/writed once, then a call
   // to vkBeginCommandBuffer will implicity reset it. It's not possible
   // to append commands to a buffer at a later time.
   auto status = vkBeginCommandBuffer(
         m_commandBuffers[commandBufferIndex],
         &beginInfo
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to begin recording command buffer!");

   VkRenderPassBeginInfo renderPassInfo{};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassInfo.renderPass = renderPass;
   // Binds the framebuffer for the swapchain image we want to draw to.
   renderPassInfo.framebuffer = framebuffer;
   // These two param. define the size of the render area. The render area
   // defines where shader loads and stores will take place. The pixels
   // outside this region will have undefined values. It should match
   // the size of the attachments for best performance.
   renderPassInfo.renderArea.offset = {0, 0};
   renderPassInfo.renderArea.extent = extent;
   // These two param. define the clear values to use for
   // VK_ATTACHMENT_LOAD_OP_CLEAR, which we used as load operation for the
   // color attachment.
   VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
   renderPassInfo.clearValueCount = 1;
   renderPassInfo.pClearValues = &clearColor;

   // It returns void, so there will be no error handling until we've finished
   // recording/writing.
   // The final parameter controls how the drawing commands withiin the render
   // pass will be provided:
   //    -VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded
   //    in the primary command buffer itself and no secondary command buffers
   //    will be executed.
   //    -VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass
   //    commands will be executed from secondary command buffers.

   //--------------------------------RenderPass--------------------------------
   vkCmdBeginRenderPass(
         m_commandBuffers[commandBufferIndex],
         &renderPassInfo,
         VK_SUBPASS_CONTENTS_INLINE
   );

   vkCmdBindPipeline(
         m_commandBuffers[commandBufferIndex],
         // this or compute pipeline
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         graphicsPipeline
   );

   // Set dynamic states
   VkViewport viewport{};
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   viewport.width = static_cast<float>(extent.width);
   viewport.height = static_cast<float>(extent.height);
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
   vkCmdSetViewport(m_commandBuffers[commandBufferIndex], 0, 1, &viewport);

   VkRect2D scissor{};
   scissor.offset = {0, 0};
   scissor.extent = extent;
   vkCmdSetScissor(m_commandBuffers[commandBufferIndex], 0, 1, &scissor);

   // Creates the draw command.
   //    - 1 param. -> VertexCount: vertices to draw.
   //    - 2 param. -> InstanceCount: For intance rendering, use 1 if we're not
   //    doint that.
   //    - 3 param. -> firstVertex: Used as an offset into the vertex buffer,
   //    defines the lowest value of gl_VertexIndex.
   //    - 4 param. -> firstIntance: Used as an offset for instanced rendering,
   //    defines the lowest value of gl_InstanceIndex.
   vkCmdDraw(m_commandBuffers[commandBufferIndex], 3, 1, 0, 0);

   vkCmdEndRenderPass(m_commandBuffers[commandBufferIndex]);

   status = vkEndCommandBuffer(m_commandBuffers[commandBufferIndex]);
   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to record command buffer!");
}

void CommandPool::resetCommandBuffer(const uint32_t commandBufferIndex)
{
   vkResetCommandBuffer(m_commandBuffers[commandBufferIndex], 0);
}

void CommandPool::createCommandBufferAllocInfo(
      VkCommandBufferAllocateInfo& allocInfo
) {
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool = m_commandPool;
   // Specifies if the allocated command buffers are primary or secondary
   // buffers:
   //    - Primary: Can be submitted to a queue for execution, but
   //    cannot be called from other command buffers.
   //    - Secondary: Cannot be submitted directly, but can be called from
   //    primary command buffers. Helpful to reuse common operations from
   //    primary command buffers.
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   // Since we are only allocation one command buffer...
   allocInfo.commandBufferCount = 1;
}

CommandPool::~CommandPool() {}
