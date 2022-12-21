#include <CroissantRenderer/VKinstance/ValidationLayers/vlManager.h>

#include <vector>
#include <iostream>
#include <cstring>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Settings/vLayersConfig.h>

/*
 * Indicates if the Vulkan call that triggered the validation layer
 * message should be aborted. If the callback returns true, then the call
 * is aborted with the VK_ERROR_VALIDATION_FAILED_EXT error.
 */
VKAPI_ATTR VkBool32 VKAPI_CALL vlManager::debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData
) {
   std::cerr << "Validation Layer: " << pCallbackData->pMessage << "\n";

   return VK_FALSE;
}

VkDebugUtilsMessengerCreateInfoEXT vlManager::getDebugMessengerCreateInfo()
{
   VkDebugUtilsMessengerCreateInfoEXT createInfo{};

   createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   createInfo.messageSeverity = (
         VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
   );
   createInfo.messageType = (
         VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
   );
   createInfo.pfnUserCallback = debugCallback;
   // Optional
   createInfo.pUserData = nullptr;

   return createInfo;
}

/*
 * The validation layers will print debug messages to the standard output by 
 * default, but we can also handle them ourselves by providing an explicit 
 * callback in our program. This will also allow us to decide which kind of 
 * messages we would like to see, because not all are necessarily (fatal) 
 * errors.
 */ 
void vlManager::setupDebugMessenger(
      VkInstance& instance,
      VkDebugUtilsMessengerEXT& debugMessenger
) {
   if (vLayersConfig::ARE_VALIDATION_LAYERS_ENABLED == false)
      return;

   VkDebugUtilsMessengerCreateInfoEXT createInfo = (
         getDebugMessengerCreateInfo()
   );
   
   auto statusOfCreation = createDebugUtilsMessengerEXT(
         instance,
         &createInfo,
         nullptr,
         &debugMessenger
   );

   if (statusOfCreation != VK_SUCCESS)
      throw std::runtime_error("Failed to set up debug messenger!");
}

bool vlManager::areAllRequestedLayersAvailable()
{
   uint32_t layerCount;
   vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

   std::vector<VkLayerProperties> availableLayers(layerCount);
   vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

   // Verifies if all required layers are available.
   // (it can be improved)
   for (const char* layerName : vLayersConfig::VALIDATION_LAYERS)
   {
      bool layerFound = false;

      for (const auto& layerProperties : availableLayers)
      {
         if (std::strcmp(layerName, layerProperties.layerName) == 0)
         {
            layerFound = true;
            break;
         }
      }

      if (!layerFound)
         return false;
   }

   return true;
}

// -------------------------------Proxies--------------------------------------

/*
 * Since the vkCreateDebugUtilsMessengerEXT functions is an extension function,
 * it's not automatically loaded, so we have to look up its address ourselves 
 * using vkGetInstanceProcAddr. 
 */
VkResult vlManager::createDebugUtilsMessengerEXT(
         VkInstance& instance,
         const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
         const VkAllocationCallbacks* pAllocator,
         VkDebugUtilsMessengerEXT* pDebugMessenger
) {
   auto func = (
         (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT"
         )
   );

   if (func != nullptr)
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
   else
      return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void vlManager::destroyDebugUtilsMessengerEXT(
      VkInstance& instance,
      VkDebugUtilsMessengerEXT& debugMessenger,
      const VkAllocationCallbacks* pAllocator
) {
    auto func = (
          (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
             instance, "vkDestroyDebugUtilsMessengerEXT"
         )
   );

    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}
