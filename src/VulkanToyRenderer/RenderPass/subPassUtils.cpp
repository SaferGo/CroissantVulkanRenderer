#include <VulkanToyRenderer/RenderPass/subPassUtils.h>

#include <vector>

#include <vulkan/vulkan.h>

/*
 * Add more parameters in the future!
 */
void subPassUtils::createSubPassDescription(
      const VkPipelineBindPoint& pipelineBindPoint,
      const VkAttachmentReference* colorAttachRef,
      const VkAttachmentReference* depthStencilAttachRef,
      const VkAttachmentReference* colorResolveAttachmentRef,
      VkSubpassDescription& subPassDescription
) {
   subPassDescription.pipelineBindPoint = pipelineBindPoint;
   subPassDescription.colorAttachmentCount = 1;
   subPassDescription.pColorAttachments = colorAttachRef;
   subPassDescription.pDepthStencilAttachment = depthStencilAttachRef;
   subPassDescription.pResolveAttachments = colorResolveAttachmentRef;
}

/*
 * Add more parameters in the future!
 */
void subPassUtils::createSubPassDependency(
      const uint32_t& srcSubPass,
      const VkPipelineStageFlags& srcStageFlags,
      const VkAccessFlags& srcAccessMask,
      const uint32_t& dstSubpass,
      const VkPipelineStageFlags& dstStageFlags,
      const VkAccessFlags& dstAccessMask,
      const VkDependencyFlagBits& dependencyFlags,
      VkSubpassDependency& dependency
) {
   // Defines the indices of the dependency and the dependent
   // subpass.
   dependency.srcSubpass = srcSubPass;

   // These two param. specify the operations to wait on and the stages in
   // which these operations occur. We need to wait for the swap chain to
   // finish reading from the image before we can access it. This can be
   // accomplished by waiting on the color attachment output stage itself.
   dependency.srcStageMask = srcStageFlags;
   dependency.srcAccessMask = srcAccessMask;
   
   // Specifies the index of the next subpass.
   dependency.dstSubpass = dstSubpass;
   // Specifies the operations that should wait. These settings will prevent
   // the transition from happenning until it's actually necessary(and allowed):
   // when we want to start writing colors to it.
   dependency.dstStageMask = dstStageFlags;
   dependency.dstAccessMask = dstAccessMask;
   dependency.dependencyFlags = dependencyFlags;
}
