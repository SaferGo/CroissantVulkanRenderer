#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace depthUtils
{
   void createDepthAttachment(
         const VkFormat& depthFormat,
         VkAttachmentDescription& depthAttachment
   );

   void createDepthAttachmentReference(
      const uint32_t index,
      VkAttachmentReference& depthAttachmentRef
   );

   void createDepthStencilStateInfo(
         VkPipelineDepthStencilStateCreateInfo& depthStencil
   );

   VkFormat findSupportedFormat(
         const VkPhysicalDevice& phyisicalDevice,
         const std::vector<VkFormat>& candidates,
         const VkImageTiling tiling,
         const VkFormatFeatureFlags features
   );
};
