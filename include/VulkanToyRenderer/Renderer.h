#pragma once

#include <vector>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <VulkanToyRenderer/Window/WindowManager.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyHandles.h>

struct Device
{
   VkDevice logicalDevice;
   VkPhysicalDevice physicalDevice;

   const std::vector<const char*> requiredExtensions = {
         VK_KHR_SWAPCHAIN_EXTENSION_NAME
   };
};

class HelloTriangleApp
{
public:

   void run();

private:

   void initWindow();
   void initVK();
   void mainLoop();
   void cleanup();

   void createVkInstance();
   void createLogicalDevice();

   std::vector<const char*> getRequiredExtensions();

   void pickPhysicalDevice();

   bool isDeviceSuitable(const VkPhysicalDevice& device);
   bool areAllExtensionsSupported(const VkPhysicalDevice& device);

   WindowManager m_windowM;
   VkInstance m_vkInstance;
   Device m_device;
   QueueFamilyIndices m_qfIndices;
   QueueFamilyHandles m_qfHandles;
   VkDebugUtilsMessengerEXT m_debugMessenger;
};
