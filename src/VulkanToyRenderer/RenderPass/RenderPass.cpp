#include <VulkanToyRenderer/RenderPass/RenderPass.h>

#include <iostream>

#include <vulkan/vulkan.h>

RenderPass::RenderPass() {}

RenderPass::~RenderPass() {}

void RenderPass::createColorAttachment(
   const VkFormat& imageFormat,
   VkAttachmentDescription& colorAttachment
) {
   colorAttachment.format = imageFormat;
   // We won't configure the multisample yet.
   colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
   // Determines what to do with the data in the attachment before rendering.
   //    - VK_ATTACHMENT_LOAD_OP_CLEAR: Specifies that the contents within the
   //    renderer area will be cleared to a uniform value, which is specified
   //    when a render pass instance is begun.
   colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   // Determines what to do with the data in the attachment after rendering.
   //    -VK_ATTACHMENT_STORE_OP_STORE: Rendered content will be stored in
   //    memory and can be read later.
   colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   // Image Layouts configuration
   // (anotar mejor su funcionamiento y para que estan)
   //
   // Specifies which layout the image will have before the render pass begins.
   //    - VK_IMAGE_LAYOUT_UNDEFINED: It means that we don't care what previous
   //    layout the image was in. The contents of the image aren't guaranteed
   //    to be preserved.
   //    (in this case we're going to clear the image anyway, so we don't
   //    care to preserve the image).
   colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   // Specifies the layout to automatically transition to when the render
   // pass finishes.
   //    - VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the
   //    swapchain.
   //    (we want the image to be ready for presentation using the swapchain
   //    after rendering)
   colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void RenderPass::createColorAttachmentReference(
      VkAttachmentReference& colorAttachmentRef
) {
   // Specifies which attachment to reference by its index in the attachment
   // descriptions array.
   // (In this case our array consists of a single VkAttachmentDescription,
   // so its index is 0)
   colorAttachmentRef.attachment = 0;
   // Specifies which layout we would like the attachment to have during a
   // subpass that uses this reference. Vulkan will automatically transition
   // the attachment to this layout when the subpass is started.
   // (In this case we intend to use the attachment to function as a color
   // buffer so the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL will give us
   // the best performance)
   colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void RenderPass::createSubPass(
      const VkAttachmentReference& colorAttachmentRef,
      VkSubpassDescription& subpassDescript
) {
   subpassDescript.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpassDescript.colorAttachmentCount = 1;
   subpassDescript.pColorAttachments = &colorAttachmentRef;
}

void RenderPass::createRenderPass(
      const VkDevice& logicalDevice,
      const VkFormat& imageFormat
) {
   // Attachments

   VkAttachmentDescription colorAttachment{};
   createColorAttachment(imageFormat, colorAttachment);

   // Attachment references

   VkAttachmentReference colorAttachmentRef{};
   createColorAttachmentReference(colorAttachmentRef);

   // Subpasses

   VkSubpassDescription subpassDescript{};
   createSubPass(colorAttachmentRef, subpassDescript);

   // Subpass dependencies

   VkSubpassDependency dependency{};
   // Defines the indices of the dependency and the dependent
   // subpass.
   //    -VK_SUBPASS_EXTERNAL: Refers to the implicit subpass before or after
   //    the render pass depending on whether is is specified in srcSubpass
   //    or dstSubpass.
   dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
   // Specifies the index of our subpass
   dependency.dstSubpass = 0;
   // These two param. specify the operations to wait on and the stages in
   // which these operations occur. We need to wait for the swap chain to
   // finish reading from the image before we can access it. This can be
   // accomplished by waiting on the color attachment output stage itself.
   dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependency.srcAccessMask = 0;
   // Specifies the operations that should wait. These settings will prevent
   // the transition from happenning until it's actually necessary(and allowed):
   // when we want to start writing colors to it.
   dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


   // Creation of the render pass

   VkRenderPassCreateInfo renderPassInfo{};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.attachmentCount = 1;
   renderPassInfo.pAttachments = &colorAttachment;
   renderPassInfo.subpassCount = 1;
   renderPassInfo.pSubpasses = &subpassDescript;
   renderPassInfo.dependencyCount= 1;
   renderPassInfo.pDependencies = &dependency;
   
   auto status = vkCreateRenderPass(
         logicalDevice,
         &renderPassInfo,
         nullptr,
         &m_renderPass
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create render pass!");
}

const VkRenderPass& RenderPass::getRenderPass() const
{
   return m_renderPass;
}

void RenderPass::destroyRenderPass(const VkDevice& logicalDevice)
{
   vkDestroyRenderPass(logicalDevice, m_renderPass, nullptr);
}
