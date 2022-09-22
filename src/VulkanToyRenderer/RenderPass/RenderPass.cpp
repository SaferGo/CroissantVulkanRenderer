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

void RenderPass::destroy(const VkDevice& logicalDevice)
{
   vkDestroyRenderPass(logicalDevice, m_renderPass, nullptr);
}

