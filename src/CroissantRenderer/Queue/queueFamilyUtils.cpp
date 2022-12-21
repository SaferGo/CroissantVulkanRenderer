#include <CroissantRenderer/Queue/queueFamilyUtils.h>

#include <vulkan/vulkan.h>

/*
 * Checks if the queue supported is a graphics queue.
 */
bool queueFamilyUtils::isGraphicsQueueSupported(
      const VkQueueFamilyProperties& qfSupported
) {
   return qfSupported.queueFlags & VK_QUEUE_GRAPHICS_BIT;
}

/*
 * Checks if the queue supported is a compute queue.
 */
bool queueFamilyUtils::isComputeQueueSupported(
      const VkQueueFamilyProperties& qfSupported
) {
   return qfSupported.queueFlags & VK_QUEUE_COMPUTE_BIT;
}

/*
 * Checks if the Queue Family is compatible with the
 * window's surface.
 */
bool queueFamilyUtils::isPresentQueueSupported(
      const int qfSupportedIndex,
      const VkSurfaceKHR& surface,
      const VkPhysicalDevice& physicalDevice
) {
   VkBool32 isSupported = false;
   vkGetPhysicalDeviceSurfaceSupportKHR(
         physicalDevice,
         qfSupportedIndex,
         surface,
         &isSupported
   );

   return isSupported;
}

/* 
 * Gets all the queue families supported by the device with their properties.
 */
void queueFamilyUtils::getSupportedQueueFamilies(
      const VkPhysicalDevice& physicalDevice,
      std::vector<VkQueueFamilyProperties>& qfSupported
) {
   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(
         physicalDevice,
         &queueFamilyCount,
         nullptr
   );

   qfSupported.resize(queueFamilyCount);
   
   // Contains some details like the type of operations that are supported and
   // the number of queues that can be created.
   vkGetPhysicalDeviceQueueFamilyProperties(
         physicalDevice,
         &queueFamilyCount,
         qfSupported.data()
   );
}
