#pragma once

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Window/Window.h>
#include <VulkanToyRenderer/DepthBuffer/DepthBuffer.h>

struct SwapchainSupportedProperties
{
   // Basic surface capabilities:
   // - Min/max number of images in swap chain.
   // - Min/max width and height of images.
   VkSurfaceCapabilitiesKHR capabilities;
   // Surface formats:
   //    - Pixel format ->
   //    - Color space  -> 
   std::vector<VkSurfaceFormatKHR> surfaceFormats;
   // Presentation modes: 
   //    The logic of how the images will be diplayed on the screen.
   std::vector<VkPresentModeKHR> presentModes;

};

class Swapchain
{
public:

   Swapchain();
   ~Swapchain();
   void createSwapchain(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const Window& window
   );
   void createAllImageViews(const VkDevice& logicalDevice);
   void createFramebuffers(
         const VkDevice& logicalDevice,
         const VkRenderPass& renderPass,
         const DepthBuffer& depthBuffer
   );

   void destroyFramebuffers(const VkDevice& logicalDevice);
   void destroySwapchain(const VkDevice& logicalDevice);
   void destroyImageViews(const VkDevice& logicalDevice);

     
   const VkExtent2D& getExtent() const;
   const VkFormat& getImageFormat() const;
   VkFramebuffer& getFramebuffer(const uint32_t imageIndex);
   VkSwapchainKHR& getSwapchain();

   // Used in isPhysicalDeviceSuitable function.
   bool isSwapchainAdequated(
      const VkPhysicalDevice& physicalDevice,
      const VkSurfaceKHR& surface
   );


private:

   void chooseBestSettings(
      const VkPhysicalDevice& physicalDevice,
      const Window& window,
      VkSurfaceFormatKHR& surfaceFormat,
      VkPresentModeKHR& presentMode,
      VkExtent2D& extent
   );
   
   VkSurfaceFormatKHR chooseBestSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats
   );
    

   VkPresentModeKHR chooseBestPresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes
   );

   VkExtent2D chooseBestExtent(
         const VkSurfaceCapabilitiesKHR& capabilities,
         const Window& window
   );

   SwapchainSupportedProperties getSupportedProperties(
      const VkPhysicalDevice& physicalDevice,
      const VkSurfaceKHR& surface
   );

   bool existsMaxNumberOfSupportedImages(
         const VkSurfaceCapabilitiesKHR& capabilities
   );

   VkSwapchainKHR m_swapchain;
   std::vector<VkImage> m_images;
   // Describes how to access the images and which part of the images to
   // access.
   std::vector<VkImageView> m_imageViews;
   // This object is "optional" beacuse we may need it before the creation
   // of the swapchain to check if the device is suitable.
   std::optional<SwapchainSupportedProperties> m_supportedProperties;
   VkFormat m_imageFormat;
   // Size of the swapchain color images.
   VkExtent2D m_extent;

   std::vector<VkFramebuffer> m_framebuffers;
};
