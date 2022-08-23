#include <VulkanToyRenderer/HelloTriangleApp.h>

#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Settings/vkLayersConfig.h>
#include <VulkanToyRenderer/ValidationLayersManager/vlManager.h>

void HelloTriangleApp::run()
{
   initWindow();
   initVK();
   mainLoop();
   cleanup();
}

void HelloTriangleApp::initWindow()
{
   glfwInit();
   // Since GLFW automatically creates an OpenGL context,
   // we need to not create it.
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   // ----------------------------------------------------------------
   // Paramet. #1 -> Width.
   // Paramet. #2 -> Height.
   // Paramet. #3 -> Title.
   // Paramet. #4 -> Specifies in which monitor to open the window.
   // Paramet. #5 -> Irrelevant to Vulkan.
   // ----------------------------------------------------------------
   m_window = glfwCreateWindow(
         config::RESOLUTION_W,
         config::RESOLUTION_H,
         config::TITLE,
         nullptr,
         nullptr
   );
}

std::vector<const char*> HelloTriangleApp::getRequiredExtensions()
{
   // - GLFW's extensions
   uint32_t glfwExtensionCount = 0;
   const char** glfwExtensions;

   glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

   std::vector<const char*> extensions(
         glfwExtensions, glfwExtensions + glfwExtensionCount
   );

   // - Vulkan Layers extensions
   if (vkLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

   return extensions;
}

void HelloTriangleApp::createVkInstance()
{
   if (vkLayersConfig::ARE_VALIDATION_LAYERS_ENABLED &&
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

   if (vkLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {

      createInfo.enabledLayerCount = static_cast<uint32_t> (
            vkLayersConfig::VALIDATION_LAYERS.size()
      );
      createInfo.ppEnabledLayerNames = vkLayersConfig::VALIDATION_LAYERS.data();

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

void HelloTriangleApp::initVK()
{
   createVkInstance();
   vlManager::setupDebugMessenger(m_vkInstance, m_debugMessenger);
}
void HelloTriangleApp::mainLoop()
{
   while (!glfwWindowShouldClose(m_window))
   {
      glfwPollEvents();
   }
}

void HelloTriangleApp::cleanup()
{
   // Validation Layers
   if (vkLayersConfig::ARE_VALIDATION_LAYERS_ENABLED)
   {
         vlManager::destroyDebugUtilsMessengerEXT(
               m_vkInstance,
               m_debugMessenger,
               nullptr
         );
   }

   // Vulkan's instance
   vkDestroyInstance(m_vkInstance, nullptr);

   // GLFW
   glfwDestroyWindow(m_window);
   glfwTerminate();
}
