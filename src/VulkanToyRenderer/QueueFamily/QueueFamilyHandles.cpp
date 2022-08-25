#include <VulkanToyRenderer/QueueFamily/QueueFamilyHandles.h>

void QueueFamilyHandles::setQueueHandles(
      const VkDevice& logicalDevice,
      const QueueFamilyIndices& qfIndices
) {
   vkGetDeviceQueue(
         logicalDevice,
         qfIndices.graphicsFamily.value(),
         0,
         &graphicsQueue
   );
   vkGetDeviceQueue(
         logicalDevice,
         qfIndices.presentFamily.value(),
         0,
         &presentQueue
   );
}

