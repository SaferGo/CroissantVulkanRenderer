#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class RenderPass
{

public:

   RenderPass();
   ~RenderPass();
   RenderPass(
      const VkDevice& logicalDevice,
      const std::vector<VkAttachmentDescription>& attachments,
      const std::vector<VkSubpassDescription>& subpasses,
      const std::vector<VkSubpassDependency>& dependencies
   );
   void begin(
      const VkFramebuffer& framebuffer,
      const VkExtent2D& extent,
      const std::vector<VkClearValue>& clearValues,
      const VkCommandBuffer& commandBuffer,
      const VkSubpassContents& subPassContents
   ) const;
   void end(const VkCommandBuffer& commandBuffer) const;
   void createSubPass(
         const VkAttachmentReference& colorAttachmentRef,
         const VkAttachmentReference& depthAttachmentRef,
         VkSubpassDescription& subpassDescript
   );

   const VkRenderPass& get() const;

   void destroy();

private:
   
   VkDevice     m_logicalDevice;
   VkRenderPass m_renderPass;

};
