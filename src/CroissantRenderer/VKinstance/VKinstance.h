#pragma once

#include <string>

#include <vulkan/vulkan.h>

class VKinstance
{

public:

   VKinstance(const std::string& appName);
   ~VKinstance();

   void destroy();
   const VkInstance& get() const;

private:

   VkInstance m_instance;
   VkDebugUtilsMessengerEXT m_debugMessenger;

};
