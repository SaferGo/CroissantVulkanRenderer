#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Window/WindowManager.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyHandles.h>
#include <VulkanToyRenderer/Swapchain/SwapchainManager.h>

struct Device
{
   VkPhysicalDevice physicalDevice;
   VkDevice logicalDevice;

   const std::vector<const char*> requiredExtensions = {
         VK_KHR_SWAPCHAIN_EXTENSION_NAME
   };
};

class HelloTriangleApp
{
public:

   void run();

private:

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
   SwapchainManager m_swapchainM;
   VkRenderPass m_renderPass;
   VkDebugUtilsMessengerEXT m_debugMessenger;
};
