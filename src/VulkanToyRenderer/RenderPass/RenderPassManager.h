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

   void createSubPass(
         const VkAttachmentReference& colorAttachmentRef,
         VkSubpassDescription& subpassDescript
   );

   const VkRenderPass& getRenderPass() const;

   void destroyRenderPass(const VkDevice& logicalDevice);

private:
   
   void createColorAttachment(
         const VkFormat& imageFormat,
         VkAttachmentDescription& colorAttachment
   );
   void createColorAttachmentReference(
         VkAttachmentReference& colorAttachmentRef
   );


   VkRenderPass m_renderPass;

};
