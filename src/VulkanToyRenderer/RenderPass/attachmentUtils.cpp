#include <VulkanToyRenderer/RenderPass/attachmentUtils.h>

#include <vulkan/vulkan.h>

void attachmentUtils::createAttachmentReference(
         const uint32_t& index,
         const VkImageLayout& imageLayout,
         VkAttachmentReference& attachmentRef
) {
   // Specifies which attachment to reference by its index in the attachment
   // descriptions array.
   attachmentRef.attachment = index;
   // Specifies which layout we would like the attachment to have during a
   // subpass that uses this reference. Vulkan will automatically transition
   // the attachment to this layout when the subpass is started.
   // (In this case we intend to use the attachment to function as a color
   // buffer so the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL will give us
   // the best performance)
   attachmentRef.layout = imageLayout;
}

void attachmentUtils::createAttachmentDescription(
   const VkFormat& imageFormat,
   const VkSampleCountFlagBits& samplesCount,
   const VkAttachmentLoadOp& colorDepthLoadOp,
   const VkAttachmentStoreOp& colorDepthStoreOp,
   const VkImageLayout& initialLayout,
   const VkImageLayout& finalLayout,
   VkAttachmentDescription& attachmentDescription
) {
   attachmentDescription.format = imageFormat;
   // We won't configure the multisample yet.
   attachmentDescription.samples = samplesCount;
   // Determines what to do with the data in the attachment before rendering.
   //    - VK_ATTACHMENT_LOAD_OP_CLEAR: Specifies that the contents within the
   //    renderer area will be cleared to a uniform value, which is specified
   //    when a render pass instance is begun.
   attachmentDescription.loadOp = colorDepthLoadOp;
   // Determines what to do with the data in the attachment after rendering.
   //    -VK_ATTACHMENT_STORE_OP_STORE: Rendered content will be stored in
   //    memory and can be read later.
   attachmentDescription.storeOp = colorDepthStoreOp;

      // Image Layouts configuration
   // (anotar mejor su funcionamiento y para que estan)
   //
   // Specifies which layout the image will have before the render pass begins.
   //    - VK_IMAGE_LAYOUT_UNDEFINED: It means that we don't care what previous
   //    layout the image was in. The contents of the image aren't guaranteed
   //    to be preserved.
   //    (in this case we're going to clear the image anyway, so we don't
   //    care to preserve the image).
   attachmentDescription.initialLayout = initialLayout;
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
   attachmentDescription.finalLayout = finalLayout;
}


void attachmentUtils::createAttachmentDescriptionWithStencil(
   const VkFormat& imageFormat,
   const VkSampleCountFlagBits& samplesCount,
   const VkAttachmentLoadOp& colorDepthLoadOp,
   const VkAttachmentStoreOp& colorDepthStoreOp,
   const VkAttachmentLoadOp& stencilLoadOp,
   const VkAttachmentStoreOp& stencilStoreOp,
   const VkImageLayout& initialLayout,
   const VkImageLayout& finalLayout,
   VkAttachmentDescription& attachmentDescription
) {
   createAttachmentDescription(
         imageFormat,
         samplesCount,
         colorDepthLoadOp,
         colorDepthStoreOp,
         initialLayout,
         finalLayout,
         attachmentDescription
   );
   attachmentDescription.stencilLoadOp = stencilLoadOp;
   attachmentDescription.stencilStoreOp = stencilStoreOp;
}
