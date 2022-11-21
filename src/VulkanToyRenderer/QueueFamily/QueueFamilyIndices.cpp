#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/QueueFamily/queueFamilyUtils.h>

/*
 * Checks if the queue families required are:
 * - Supported by the device.
 * - Supported by the window's surface(in the case of the "Present" qf).
 * If they do, their indices are stored.
 */
void QueueFamilyIndices::getIndicesOfRequiredQueueFamilies(
         const VkPhysicalDevice& physicalDevice,
         const VkSurfaceKHR& surface
) {
   std::vector<VkQueueFamilyProperties> qfSupported;
   queueFamilyUtils::getSupportedQueueFamilies(physicalDevice, qfSupported);

   int i = 0;
   for (const auto& qf : qfSupported)
   {
      if (queueFamilyUtils::isGraphicsQueueSupported(qf))
         graphicsFamily = i;

      if (queueFamilyUtils::isPresentQueueSupported(i, surface, physicalDevice))
         presentFamily = i;

      i++;
   }

   // Verifies if all the QF required are supported.
   areAllQueueFamiliesSupported = (
         graphicsFamily.has_value() &&
         presentFamily.has_value()
   );
}

