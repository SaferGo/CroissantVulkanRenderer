#pragma once

#include <vulkan/vulkan.h>

namespace commandUtils
{
   // Methods used to record the cmd buffer used to copy a buffer to the
   // gpu's buffer.
   namespace copyCommandBuffer{
      void record(
         const VkDeviceSize size,
         VkBuffer& srcBuffer,
         VkBuffer& dstBuffer,
         VkCommandBuffer& commandBuffer
      );
   };
};
