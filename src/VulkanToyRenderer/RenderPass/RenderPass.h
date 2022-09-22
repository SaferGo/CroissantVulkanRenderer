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
   void createSubPass(
         const VkAttachmentReference& colorAttachmentRef,
         const VkAttachmentReference& depthAttachmentRef,
         VkSubpassDescription& subpassDescript
   );

   const VkRenderPass& get() const;

   void destroy(const VkDevice& logicalDevice);

private:
   
   VkRenderPass m_renderPass;

};
