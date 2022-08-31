#pragma once

#include <vulkan/vulkan.h>

class RenderPassManager
{

public:

   RenderPassManager();
   ~RenderPassManager();

   void createRenderPass(
         const VkDevice& logicalDevice,
         const VkFormat& imageFormat
   );

   void createColorAttachment(
         const VkFormat& imageFormat,
         VkAttachmentDescription& colorAttachment
   );
   void createColorAttachmentReference(
         VkAttachmentReference& colorAttachmentRef
   );

   void createSubPass(
         const VkAttachmentReference& colorAttachmentRef,
         VkSubpassDescription& subpass
   );

   const VkRenderPass& getRenderPass() const;

   void destroyRenderPass(const VkDevice& logicalDevice);

private:

   VkRenderPass m_renderPass;

};
