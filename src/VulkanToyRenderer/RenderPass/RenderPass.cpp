#include <VulkanToyRenderer/RenderPass/RenderPass.h>

#include <iostream>
#include <array>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/DepthBuffer/depthUtils.h>

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
   //    CHANGE: Since the last render pass will be the IMGUI(porq se tiene q
   //    renderizar al frente de todo), this render pass won't present! That's
   //    why we changed VK_IMAGE_LAYOUT_PRESENT+SRC_KHR to
   //    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
   colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void RenderPass::createColorAttachmentReference(
      const uint32_t index,
      VkAttachmentReference& colorAttachmentRef
) {
   // Specifies which attachment to reference by its index in the attachment
   // descriptions array.
   // (In this case our array consists of a single VkAttachmentDescription,
   // so its index is 0)
   colorAttachmentRef.attachment = index;
   std::cout << "ASIGNADO" << colorAttachmentRef.attachment << " COLOR \n";
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
      const VkAttachmentReference& depthAttachmentRef,
      VkSubpassDescription& subpassDescript
) {
   subpassDescript.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpassDescript.colorAttachmentCount = 1;
   subpassDescript.pColorAttachments = &colorAttachmentRef;
   subpassDescript.pDepthStencilAttachment = &depthAttachmentRef;
}

void RenderPass::createRenderPass(
      const VkPhysicalDevice& phyisicalDevice,
      const VkDevice& logicalDevice,
      const VkFormat& imageFormat
) {
   // - Attachments

   // Color Attachment
   VkAttachmentDescription colorAttachment{};
   createColorAttachment(imageFormat, colorAttachment);

   // Depth Attachment
   VkFormat depthFormat = depthUtils::findSupportedFormat(
         phyisicalDevice,
         {
          VK_FORMAT_D32_SFLOAT,
          VK_FORMAT_D32_SFLOAT_S8_UINT,
          VK_FORMAT_D24_UNORM_S8_UINT
         },
         VK_IMAGE_TILING_OPTIMAL,
         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
   );
   VkAttachmentDescription depthAttachment{};
   depthUtils::createDepthAttachment(
         depthFormat,
         depthAttachment
   );

   // Attachment references

   VkAttachmentReference colorAttachmentRef{};
   createColorAttachmentReference(0, colorAttachmentRef);

   VkAttachmentReference depthAttachmentRef{};
   depthUtils::createDepthAttachmentReference(
         1,
         depthAttachmentRef
   );

   // Subpasses

   VkSubpassDescription subpassDescript{};
   createSubPass(colorAttachmentRef, depthAttachmentRef, subpassDescript);

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
   // (Lo actualize cuando agregue el depth buffer...)
   dependency.srcStageMask = (
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
   );
   dependency.srcAccessMask = 0;
   // Specifies the operations that should wait. These settings will prevent
   // the transition from happenning until it's actually necessary(and allowed):
   // when we want to start writing colors to it.
   // (Lo actualize cuando agregue el depth buffer...)
   dependency.dstStageMask = (
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
   );
   // (Lo actualize cuando agregue el depth buffer...)
   dependency.dstAccessMask = (
         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
   );


   // Creation of the render pass
   std::array<VkAttachmentDescription, 2> attachments = {
      colorAttachment,
      depthAttachment
   };

   VkRenderPassCreateInfo renderPassInfo{};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
   renderPassInfo.pAttachments = attachments.data();
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

