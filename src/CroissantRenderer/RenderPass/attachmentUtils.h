#pragma once

#include <vulkan/vulkan.h>

namespace attachmentUtils
{
   void createAttachmentReference(
         const uint32_t& index,
         const VkImageLayout& imageLayout,
         VkAttachmentReference& attachmentRef
   );
   void createAttachmentDescription(
      const VkFormat& imageFormat,
      const VkSampleCountFlagBits& samplesCount,
      const VkAttachmentLoadOp& colorDepthLoadOp,
      const VkAttachmentStoreOp& colorDepthStoreOp,
      const VkImageLayout& initialLayout,
      const VkImageLayout& finalLayout,
      VkAttachmentDescription& attachmentDescription
   );
   void createAttachmentDescriptionWithStencil(
      const VkFormat& imageFormat,
      const VkSampleCountFlagBits& samplesCount,
      const VkAttachmentLoadOp& colorDepthLoadOp,
      const VkAttachmentStoreOp& colorDepthStoreOp,
      const VkAttachmentLoadOp& stencilLoadOp,
      const VkAttachmentStoreOp& stencilStoreOp,
      const VkImageLayout& initialLayout,
      const VkImageLayout& finalLayout,
      VkAttachmentDescription& attachmentDescription
   );
};
