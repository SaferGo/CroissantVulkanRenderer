#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace framebufferUtils
{
   void createFramebuffer(
         const VkDevice& logicalDevice,
         const VkRenderPass& renderPass,
         std::vector<VkImageView>& attachments,
         const uint32_t width,
         const uint32_t height,
         const uint32_t layersCount,
         VkFramebuffer& framebuffer
   );
};
