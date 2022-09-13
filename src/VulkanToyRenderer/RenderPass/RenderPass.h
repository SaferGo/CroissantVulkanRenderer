#pragma once

#include <vulkan/vulkan.h>

class RenderPass
{

public:

   RenderPass();
   ~RenderPass();

   void createRenderPass(
         const VkPhysicalDevice& phyisicalDevice,
         const VkDevice& logicalDevice,
         const VkFormat& imageFormat
   );

   void createSubPass(
         const VkAttachmentReference& colorAttachmentRef,
         const VkAttachmentReference& depthAttachmentRef,
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
         const uint32_t index,
         VkAttachmentReference& colorAttachmentRef
   );

   VkRenderPass m_renderPass;

};
