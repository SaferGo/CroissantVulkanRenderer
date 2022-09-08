#include <VulkanToyRenderer/Commands/CommandPool.h>

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>

CommandPool::CommandPool(
      const VkDevice& logicalDevice,
      QueueFamilyIndices& queueFamilyIndices
) {
   m_logicalDevice = logicalDevice;
   m_queueFamilyIndices = queueFamilyIndices;
   m_commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);

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

void CommandPool::allocCommandBuffer(VkCommandBuffer& commandBuffer)
{
   VkCommandBufferAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandPool = m_commandPool;
   allocInfo.commandBufferCount = 1;

   vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);
}

/*
 * Allocates all the commands buffers saved in m_commandBuffers.
*/
void CommandPool::allocAllCommandBuffers()
{
   if (m_commandBuffers.size() == 0)
      throw std::runtime_error("Allocating empty CMD Buffers to the CMD Pool!");

   VkCommandBufferAllocateInfo allocInfo {};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool = m_commandPool;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

   auto status = vkAllocateCommandBuffers(
         m_logicalDevice,
         &allocInfo,
         m_commandBuffers.data()
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate command buffers!");
}

const VkCommandBuffer& CommandPool::getCommandBuffer(
      const uint32_t index
) {
   return m_commandBuffers[index];
}

void CommandPool::recordCommandBuffer(
      const VkFramebuffer& framebuffer,
      const VkRenderPass& renderPass,
      const VkExtent2D& extent,
      const VkPipeline& graphicsPipeline,
      const uint32_t index,
      const VkBuffer& vertexBuffer,
      const VkBuffer& indexBuffer,
      const size_t vertexCount,
      VkPipelineLayout& pipelineLayout,
      const std::vector<VkDescriptorSet>& descriptorSets
) {
   // Specifies some details about the usage of this specific command
   // buffer.
   VkCommandBufferBeginInfo beginInfo{};
   createCommandBufferBeginInfo(m_commandBuffers[index], beginInfo);
   

   VkRenderPassBeginInfo renderPassInfo{};
   createRenderPassBeginInfo(renderPass, framebuffer, extent, renderPassInfo);
   
   //--------------------------------RenderPass--------------------------------

   // The final parameter controls how the drawing commands between the render
   // pass will be provided:
   //    -VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded
   //    in the primary command buffer itself and no secondary command buffers
   //    will be executed.
   //    -VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass
   //    commands will be executed from secondary command buffers.
   vkCmdBeginRenderPass(
         m_commandBuffers[index],
         &renderPassInfo,
         VK_SUBPASS_CONTENTS_INLINE
   );

   vkCmdBindPipeline(
         m_commandBuffers[index],
         // this or compute pipeline
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         graphicsPipeline
   );

   bindVertexBuffers(vertexBuffer, m_commandBuffers[index]);
   bindIndexBuffer(indexBuffer, m_commandBuffers[index]);
   // Set Dynamic States
   setViewport(extent, m_commandBuffers[index]);
   setScissor(extent, m_commandBuffers[index]);

   // Binds the right descriptor set to the descriptors in the shader.
   vkCmdBindDescriptorSets(
         m_commandBuffers[index],
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         pipelineLayout,
         // Index of the first descriptor set.
         0,
         // The number of sets to bind.
         1,
         &descriptorSets[index],
         // Array of offsets that are used for dynamic descriptors.
         0,
         nullptr
   );
   
   // Creates the draw command.
   //    - 3 param. -> InstanceCount.
   //    - 4 param. -> Offset(in the index buffer).
   //    - 5 param. -> Vertex offset.
   //    - 6 param. -> Instance offset.
   vkCmdDrawIndexed(
         m_commandBuffers[index],
         vertexCount,
         1,
         0,
         0,
         0
   );

   vkCmdEndRenderPass(m_commandBuffers[index]);

   auto status = vkEndCommandBuffer(m_commandBuffers[index]);
   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to record command buffer!");
}

void CommandPool::createRenderPassBeginInfo(
      const VkRenderPass& renderPass,
      const VkFramebuffer& framebuffer,
      const VkExtent2D& extent,
      VkRenderPassBeginInfo& renderPassInfo
) {
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
   renderPassInfo.clearValueCount = 1;
   renderPassInfo.pClearValues = &config::CLEAR_COLOR;
}

void CommandPool::bindVertexBuffers(
      const VkBuffer& vertexBuffer,
      VkCommandBuffer& commandBuffer
) {
   VkBuffer vertexBuffers[] = {vertexBuffer};
   VkDeviceSize offsets[] = {0};

   vkCmdBindVertexBuffers(
         commandBuffer,
         // Offset
         0,
         // Number of bindings.
         1,
         vertexBuffers,
         // Data offset
         offsets
   );
}

void CommandPool::bindIndexBuffer(
      const VkBuffer& indexBuffer,
      VkCommandBuffer& commandBuffer
) {
   vkCmdBindIndexBuffer(
         commandBuffer,
         indexBuffer,
         0,
         VK_INDEX_TYPE_UINT16
   );
}


void CommandPool::setViewport(
      const VkExtent2D& extent,
      VkCommandBuffer& commandBuffer
) {
   VkViewport viewport{};
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   viewport.width = static_cast<float>(extent.width);
   viewport.height = static_cast<float>(extent.height);
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
   vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void CommandPool::setScissor(
      const VkExtent2D& extent,
      VkCommandBuffer& commandBuffer
) {
   VkRect2D scissor{};
   scissor.offset = {0, 0};
   scissor.extent = extent;
   vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void CommandPool::createCommandBufferBeginInfo(
      VkCommandBuffer& commandBuffer,
      VkCommandBufferBeginInfo &beginInfo
) {
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
         commandBuffer,
         &beginInfo
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to begin recording command buffer!");
}

void CommandPool::resetCommandBuffer(const uint32_t index)
{
   vkResetCommandBuffer(m_commandBuffers[index], 0);
}

void CommandPool::resetCommandBuffer(VkCommandBuffer& commandBuffer)
{
   vkResetCommandBuffer(commandBuffer, 0);
}

void CommandPool::freeCommandBuffer(VkCommandBuffer& commandBuffer)
{
   vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
}

CommandPool::~CommandPool() {}
