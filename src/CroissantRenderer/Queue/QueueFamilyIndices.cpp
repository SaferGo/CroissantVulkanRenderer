#include <CroissantRenderer/Queue/QueueFamilyIndices.h>

#include <vector>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Queue/queueFamilyUtils.h>

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

      if (queueFamilyUtils::isComputeQueueSupported(qf))
         computeFamily = i;

      i++;
   }

   // TODO: Check if the graphicsFamily index is the same as the Compute Family
   // index. This is the case for all devices.

   // Verifies if all the QF required are supported.
   areAllQueueFamiliesSupported = (
         graphicsFamily.has_value() &&
         presentFamily.has_value() &&
         computeFamily.has_value()
   );
}

