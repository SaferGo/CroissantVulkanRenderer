#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/Swapchain/Swapchain.h>

class Device
{

public:
   
   Device();
   ~Device();
   void createLogicalDevice(QueueFamilyIndices& requiredQueueFamiliesIndices);
   void pickPhysicalDevice(
         VkInstance& m_vkInstance,
         QueueFamilyIndices& requiredQueueFamiliesIndices,
         const VkSurfaceKHR& windowSurface,
         Swapchain& swapchain
   );
   const VkDevice& getLogicalDevice();
   const VkPhysicalDevice& getPhysicalDevice();

private:

   bool isPhysicalDeviceSuitable(
      QueueFamilyIndices& requiredQueueFamiliesIndices,
      const VkSurfaceKHR& windowSurface,
      Swapchain& swapchain,
      const VkPhysicalDevice& possiblePhysicalDevice
   );
   bool areAllExtensionsSupported(
         const VkPhysicalDevice& possiblePhysicalDevice
   );


   VkPhysicalDevice m_physicalDevice;
   VkDevice m_logicalDevice;
   const std::vector<const char*> m_requiredExtensions = {
         VK_KHR_SWAPCHAIN_EXTENSION_NAME
   };
};
