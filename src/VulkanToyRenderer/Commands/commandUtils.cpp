#include <VulkanToyRenderer/Commands/commandUtils.h>

void commandUtils::copyCommandBuffer::record(
      const VkDeviceSize size,
      VkBuffer& srcBuffer,
      VkBuffer& dstBuffer,
      VkCommandBuffer& commandBuffer
) {
   VkCommandBufferBeginInfo beginInfo{};
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

   vkBeginCommandBuffer(commandBuffer, &beginInfo);
      
      VkBufferCopy copyRegion{};
      // optional
      copyRegion.srcOffset = 0;
      // optional
      copyRegion.dstOffset = 0;
      copyRegion.size = size;
      vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
   
   vkEndCommandBuffer(commandBuffer);
}
