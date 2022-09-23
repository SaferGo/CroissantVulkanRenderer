#include <VulkanToyRenderer/RenderPass/RenderPass.h>

#include <iostream>
#include <array>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/DepthBuffer/depthUtils.h>

RenderPass::RenderPass() {}
RenderPass::~RenderPass() {}

RenderPass::RenderPass(
   const VkDevice& logicalDevice,
   const std::vector<VkAttachmentDescription>& attachments,
   const std::vector<VkSubpassDescription>& subpasses,
   const std::vector<VkSubpassDependency>& dependencies
) {
   VkRenderPassCreateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   info.attachmentCount = attachments.size();
   info.pAttachments = attachments.data();
   info.subpassCount = subpasses.size();
   info.pSubpasses = subpasses.data();
   info.dependencyCount = dependencies.size();
   info.pDependencies = dependencies.data();

   auto status = vkCreateRenderPass(
         logicalDevice,
         &info,
         nullptr,
         &m_renderPass
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create Imgui's render pass");

}

const VkRenderPass& RenderPass::get() const
{
   return m_renderPass;
}

void RenderPass::begin(
      const VkFramebuffer& framebuffer,
      const VkExtent2D& extent,
      const std::vector<VkClearValue>& clearValues,
      const VkCommandBuffer& commandBuffer,
      const VkSubpassContents& subPassContents
) const {
   VkRenderPassBeginInfo renderPassInfo{};

   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassInfo.renderPass = m_renderPass;
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

   vkCmdBeginRenderPass(
         commandBuffer,
         &renderPassInfo,
         // Specifies how the commands in the first subpass will be provided.
         subPassContents
   );
}
         
void RenderPass::end(const VkCommandBuffer& commandBuffer) const
{
   vkCmdEndRenderPass(commandBuffer);
}

void RenderPass::destroy(const VkDevice& logicalDevice)
{
   vkDestroyRenderPass(logicalDevice, m_renderPass, nullptr);
}

