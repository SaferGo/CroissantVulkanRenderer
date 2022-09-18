#include <VulkanToyRenderer/Commands/CommandPool.h>

#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/Commands/commandUtils.h>

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

void CommandPool::submitCommandBuffer(
      VkQueue& graphicsQueue,
      VkCommandBuffer& commandBuffer
) {
   VkSubmitInfo submitInfo{};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &commandBuffer;

   // Submits and execute the cmd immediately and wait on this transfer to
   // complete.
   vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
   vkQueueWaitIdle(graphicsQueue);

   freeCommandBuffer(commandBuffer);
}

/*
 * Allocates all the commands buffers saved in m_commandBuffers in the 
 * cmd pool.
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

VkCommandBuffer& CommandPool::getCommandBuffer(const uint32_t index)
{
   return m_commandBuffers[index];
}

void CommandPool::createRenderPassBeginInfo(
      const VkRenderPass& renderPass,
      const VkFramebuffer& framebuffer,
      const VkExtent2D& extent,
      const std::vector<VkClearValue>& clearValues,
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
   renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
   renderPassInfo.pClearValues = clearValues.data();
}

void CommandPool::beginCommandBuffer(
      const VkCommandBufferUsageFlags& flags,
      VkCommandBuffer& commandBuffer
) {
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
   beginInfo.flags = flags;
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

void CommandPool::endCommandBuffer(VkCommandBuffer& commandBuffer)
{
   auto status = vkEndCommandBuffer(commandBuffer);
   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to record command buffer!");
}

void CommandPool::resetCommandBuffer(const uint32_t index)
{
   vkResetCommandBuffer(m_commandBuffers[index], 0);
}


void CommandPool::freeCommandBuffer(VkCommandBuffer& commandBuffer)
{
   vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
}

CommandPool::~CommandPool() {}
