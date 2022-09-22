#include <VulkanToyRenderer/RenderPass/subPassUtils.h>

#include <vector>

#include <vulkan/vulkan.h>

/*
 * Add more parameters in the future!
 */
void subPassUtils::createSubPassDescription(
      const VkPipelineBindPoint& pipelineBindPoint,
      const std::vector<VkAttachmentReference>& colorAttachRefs,
      const VkAttachmentReference* depthStencilAttachRef,
      VkSubpassDescription& subPassDescription
) {
   subPassDescription.pipelineBindPoint = pipelineBindPoint;
   subPassDescription.colorAttachmentCount = colorAttachRefs.size();
   subPassDescription.pColorAttachments = colorAttachRefs.data();
   subPassDescription.pDepthStencilAttachment = depthStencilAttachRef;
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
}
