#include <VulkanToyRenderer/Renderer.h>

#include <iostream>
#include <vector>
#include <set>
#include <cstring>
#include <limits>
#include <algorithm>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Settings/vLayersConfig.h>
#include <VulkanToyRenderer/Window/WindowManager.h>
#include <VulkanToyRenderer/ValidationLayers/vlManager.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>
#include <VulkanToyRenderer/QueueFamily/QueueFamilyHandles.h>
#include <VulkanToyRenderer/Swapchain/SwapchainManager.h>

void HelloTriangleApp::run()
{  
   m_windowM.createWindow(
         config::RESOLUTION_W,
         config::RESOLUTION_H,
         config::TITLE
   );
   initVK();
   mainLoop();
   cleanup();
}

std::vector<const char*> HelloTriangleApp::getRequiredExtensions()
{
   std::vector<const char*> extensions;

   // - GLFW's extensions
   uint32_t glfwExtensionCount = 0;
   const char** glfwExtensions;

   glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
   for (uint32_t i = 0; i != glfwExtensionCount; i++)
      extensions.push_back(*(glfwExtensions + i));

   // - Vulkan Layers extensions
   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

   return extensions;
}

void HelloTriangleApp::createVkInstance()
{
   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED &&
       !vlManager::areAllRequestedLayersAvailable()
   ) {
      throw std::runtime_error(
            "Validation layers requested, but not available!"
      );
   }

   // This data is optional, but it may provide some useful information
   // to the driver in order to optimize our specific application(e.g because
   // it uses a well-known graphics engine with certain special behavior).
   //
   // Example: Our game uses UE and nvidia launched a new driver that optimizes
   // a certain thing. So in that case, nvidia will know it can apply that
   // optimization verifying this info.
   VkApplicationInfo appInfo{};

   appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pApplicationName = config::TITLE;
   appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
   appInfo.pEngineName = "No Engine";
   appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
   appInfo.apiVersion = VK_API_VERSION_1_0;

   // This data is not optional and tells the Vulkan driver which global
   // extensions and validation layers we want to use.
   VkInstanceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   createInfo.pApplicationInfo = &appInfo;

   std::vector<const char*> extensions = getRequiredExtensions();

   createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
   createInfo.ppEnabledExtensionNames = extensions.data();

   // This variable is placed outside the if statement to ensure that it is
   // not destroyed before the vkCreateInstance call.
   VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {

      createInfo.enabledLayerCount = static_cast<uint32_t> (
            vLayersConfig::VALIDATION_LAYERS.size()
      );
      createInfo.ppEnabledLayerNames = vLayersConfig::VALIDATION_LAYERS.data();

      debugCreateInfo = vlManager::getDebugMessengerCreateInfo();

      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;

   } else
   {
      createInfo.enabledLayerCount = 0;

      createInfo.pNext = nullptr;
   }


   // -------------------------------------------------------------
   // Paramet. 1 -> Pointer to struct with creation info.
   // Paramet. 2 -> Pointer to custom allocator callbacks.
   // Paramet. 3 -> Pointer to the variable that stores the handle to the
   //               new object.
   // -------------------------------------------------------------
   if (vkCreateInstance(&createInfo, nullptr, &m_vkInstance) != VK_SUCCESS)
      throw std::runtime_error("Failed to create Vulkan's instance!");
}

bool HelloTriangleApp::areAllExtensionsSupported(
      const VkPhysicalDevice& device
) {
   uint32_t extensionCount;
   vkEnumerateDeviceExtensionProperties(
         device,
         nullptr,
         &extensionCount,
         nullptr
   );

   std::vector<VkExtensionProperties> availableExtensions(extensionCount);
   vkEnumerateDeviceExtensionProperties(
         device,
         nullptr,
         &extensionCount,
         availableExtensions.data()
   );

   // Verifies if all required extensions are available in the device.
   // (it can be improved)
   for (const auto& requiredExtension : m_device.requiredExtensions)
   {
      bool extensionFound = false;

      for (const auto& availableExtension : availableExtensions)
      {
         const char* extensionName = availableExtension.extensionName;
         if (std::strcmp(requiredExtension, extensionName) == 0)
         {
            extensionFound = true;
            break;
         }
      }

      if (extensionFound == false)
         return false;
   }

   return true;
}

bool HelloTriangleApp::isDeviceSuitable(const VkPhysicalDevice& physicalDevice)
{
   // - Queue-Families
   // Verifies if the device has the Queue families that we need.
   m_qfIndices.getIndicesOfRequiredQueueFamilies(
         physicalDevice, m_windowM.getSurface()
   );

   if (m_qfIndices.areAllQueueFamiliesSupported() == false)
      return false;
   
   // - Device Properties
   // Verifies if the device has the properties we want.
   VkPhysicalDeviceProperties deviceProperties;
   // Gives us basic device properties like the name, type and supported
   // Vulkan version.
   vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

   // - Device Features
   // Verifies if the device has the features we want.
   VkPhysicalDeviceFeatures deviceFeatures;
   // Tells us if features like texture compression, 64 bit floats, multi
   // vieport renderending and so on, are compatbile with this device.
   vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

   // Here we can score the gpu(so later select the best one to use) or just
   // verify if it has the features that we need.

   // For now, we will just return the dedicated one.
   if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      return false;

   // - Device Extensions
   if (areAllExtensionsSupported(physicalDevice) == false)
      return false;

   // - Swapchain support
   if (m_swapchainM.isSwapchainAdequated(
            physicalDevice, m_windowM.getSurface()
      ) == false
   ) {
      return false;
   }

   return true;
}

void HelloTriangleApp::pickPhysicalDevice()
{
      uint32_t deviceCount = 0;
      vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

      if (deviceCount == 0)
         throw std::runtime_error("Failed to find GPUs with Vulkan support!");

      std::vector<VkPhysicalDevice> devices(deviceCount);
      vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

      for (const auto& device : devices)
      {
         if (isDeviceSuitable(device))
         {
            m_device.physicalDevice = device;
            break;
         }
      }

      if (m_device.physicalDevice == VK_NULL_HANDLE)
         throw std::runtime_error("Failed to find a suitable GPU!");
}

void HelloTriangleApp::createLogicalDevice()
{
   // - Specifies which QUEUES we want to create.
   std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   // We use a set because we need to not repeat them to create the new one
   // for the logical device.
   std::set<uint32_t> uniqueQueueFamilies = {
         m_qfIndices.graphicsFamily.value(),
         m_qfIndices.presentFamily.value()
   };

   float queuePriority = 1.0f;
   for (uint32_t queueFamily : uniqueQueueFamilies)
   {
      VkDeviceQueueCreateInfo queueCreateInfo{};

      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;

      queueCreateInfos.push_back(queueCreateInfo);
   }

   // - Specifices which device FEATURES we want to use.
   // For now, this will be empty.
   VkPhysicalDeviceFeatures deviceFeatures{};


   // Now we can create the logical device.
   VkDeviceCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   createInfo.queueCreateInfoCount = static_cast<uint32_t>(
         queueCreateInfos.size()
   );
   createInfo.pQueueCreateInfos = queueCreateInfos.data();
   createInfo.pEnabledFeatures = &deviceFeatures;

   // - Specifies which device EXTENSIONS we want to use.
   createInfo.enabledExtensionCount = static_cast<uint32_t>(
         m_device.requiredExtensions.size()
   );

   createInfo.ppEnabledExtensionNames = m_device.requiredExtensions.data();

   // Previous implementations of Vulkan made a distinction between instance 
   // and device specific validation layers, but this is no longer the 
   // case. That means that the enabledLayerCount and ppEnabledLayerNames 
   // fields of VkDeviceCreateInfo are ignored by up-to-date 
   // implementations. However, it is still a good idea to set them anyway to 
   // be compatible with older implementations:

   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {
      createInfo.enabledLayerCount = static_cast<uint32_t>(
            vLayersConfig::VALIDATION_LAYERS.size()
      );
      createInfo.ppEnabledLayerNames = (
            vLayersConfig::VALIDATION_LAYERS.data()
      );
   } else
      createInfo.enabledLayerCount = 0;

   auto status = vkCreateDevice(
         m_device.physicalDevice,
         &createInfo,
         nullptr,
         &m_device.logicalDevice
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create logical device!");

   m_qfHandles.setQueueHandles(m_device.logicalDevice, m_qfIndices);
}

void createGraphicsPipeline()
{

}

void HelloTriangleApp::initVK()
{
   createVkInstance();

   vlManager::setupDebugMessenger(m_vkInstance, m_debugMessenger);
   m_windowM.createSurface(m_vkInstance);

   pickPhysicalDevice();
   createLogicalDevice();

   m_swapchainM.createSwapchain(
         m_device.physicalDevice,
         m_device.logicalDevice,
         m_windowM
   );

   m_swapchainM.createImageViews(m_device.logicalDevice);

   createGraphicsPipeline();
}
void HelloTriangleApp::mainLoop()
{
   while (m_windowM.isWindowClosed() == false)
   {
      m_windowM.pollEvents();
   }
}

void HelloTriangleApp::cleanup()
{
   // Swapchain
   m_swapchainM.destroySwapchain(m_device.logicalDevice);

   // ViewImages of the images from the Swapchain
   m_swapchainM.destroyImageViews(m_device.logicalDevice);

   // Logical Device
   vkDestroyDevice(m_device.logicalDevice, nullptr);

   // Validation Layers
   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {
         vlManager::destroyDebugUtilsMessengerEXT(
               m_vkInstance,
               m_debugMessenger,
               nullptr
         );
   }

   // Window Surface
   m_windowM.destroySurface(m_vkInstance);

   // Vulkan's instance
   vkDestroyInstance(m_vkInstance, nullptr);

   // GLFW
   m_windowM.destroyWindow();
}
