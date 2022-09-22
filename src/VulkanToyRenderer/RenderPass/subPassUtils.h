#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace subPassUtils
{
   void createSubPassDescription(
         const VkPipelineBindPoint& pipelineBindPoint,
         const std::vector<VkAttachmentReference>& colorAttachRefs,
         const VkAttachmentReference* depthStencilAttachRef,
         VkSubpassDescription& subPassDescription
   );

   void createSubPassDependency(
         const uint32_t& srcSubPass,
         const VkPipelineStageFlags& srcStageFlags,
         const VkAccessFlags& srcAccessMask,
         const uint32_t& dstSubPass,
         const VkPipelineStageFlags& dstStageFlags,
         const VkAccessFlags& dstAccessMask,
         VkSubpassDependency& dependency
   );

};
