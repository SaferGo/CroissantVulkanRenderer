#include <CroissantRenderer/Framebuffer/framebufferManager.h>

#include <vector>
#include <iostream>

#include <vulkan/vulkan.h>

void framebufferManager::createFramebuffer(
      const VkDevice& logicalDevice,
      const VkRenderPass& renderPass,
      std::vector<VkImageView>& attachments,
      const uint32_t width,
      const uint32_t height,
      const uint32_t layersCount,
      VkFramebuffer& framebuffer
) {

   VkFramebufferCreateInfo framebufferInfo{};
   framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   framebufferInfo.renderPass = renderPass;
   framebufferInfo.attachmentCount = static_cast<uint32_t>(
         attachments.size()
   );
   framebufferInfo.pAttachments = attachments.data();
   framebufferInfo.width = width;
   framebufferInfo.height = height;
   framebufferInfo.layers = layersCount;
   
   auto status = vkCreateFramebuffer(
         logicalDevice,
         &framebufferInfo,
         nullptr,
         &framebuffer
   );
   
   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create framebuffer!");
}
