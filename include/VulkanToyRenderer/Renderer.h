#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Window/WindowManager.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyHandles.h>
#include <VulkanToyRenderer/Swapchain/SwapchainManager.h>
#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipelineManager.h>
#include <VulkanToyRenderer/RenderPass/RenderPassManager.h>
#include <VulkanToyRenderer/Commands/CommandManager.h>

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
   void drawFrame();

   void createVkInstance();
   void createLogicalDevice();
   void createSyncObjects();

   std::vector<const char*> getRequiredExtensions();

   void pickPhysicalDevice();

   bool isDeviceSuitable(const VkPhysicalDevice& device);
   bool areAllExtensionsSupported(const VkPhysicalDevice& device);

   void destroySyncObjects();

   WindowManager            m_windowM;
   VkInstance               m_vkInstance;
   Device                   m_device;
   QueueFamilyIndices       m_qfIndices;
   QueueFamilyHandles       m_qfHandles;
   SwapchainManager         m_swapchainM;
   RenderPassManager        m_renderPassM;
   GraphicsPipelineManager  m_graphicsPipelineM;
   VkDebugUtilsMessengerEXT m_debugMessenger;
   CommandManager           m_commandM;

   // Sync objects
   VkSemaphore m_imageAvailableSemaphore;
   VkSemaphore m_renderFinishedSemaphore;
   VkFence m_inFlightFence;
};
