#include <VulkanToyRenderer/QueueFamily/qfUtils.h>

#include <vulkan/vulkan.h>

/*
 * Checks if the queue supported is a graphic's queue.
 */
bool qfUtils::isGraphicsQueueSupported(
      const VkQueueFamilyProperties& qfSupported
) {
   return qfSupported.queueFlags & VK_QUEUE_GRAPHICS_BIT;
}

/*
 * Checks if the Queue Family is compatible with the
 * window's surface.
 */
bool qfUtils::isPresentQueueSupported(
      const int qfSupportedIndex,
      const VkSurfaceKHR& surface,
      const VkPhysicalDevice& device
) {
   VkBool32 isSupported = false;
   vkGetPhysicalDeviceSurfaceSupportKHR(
         device,
         qfSupportedIndex,
         surface,
         &isSupported
   );

   return isSupported;
}

/* 
 * Gets all the queue families supported by the device with their properties.
 */
void qfUtils::getSupportedQueueFamilies(
      const VkPhysicalDevice& device,
      std::vector<VkQueueFamilyProperties>& qfSupported
) {
   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties(
         device,
         &queueFamilyCount,
         nullptr
   );

   qfSupported.resize(queueFamilyCount);
   
   // Contains some details like the type of operations that are supported and
   // the number of queues that can be created.
   vkGetPhysicalDeviceQueueFamilyProperties(
         device,
         &queueFamilyCount,
         qfSupported.data()
   );
}
