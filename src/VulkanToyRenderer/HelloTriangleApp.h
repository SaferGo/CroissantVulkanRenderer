#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

   std::vector<const char*> getRequiredExtensions();
   
   GLFWwindow* m_window;
   VkInstance m_vkInstance;
   VkDebugUtilsMessengerEXT m_debugMessenger;
};
