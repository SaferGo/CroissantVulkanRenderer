#pragma once

#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Window/Window.h>
#include <CroissantRenderer/Features/MSAA.h>
#include <CroissantRenderer/Features/DepthBuffer.h>
#include <CroissantRenderer/RenderPass/RenderPass.h>

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
   Swapchain(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const std::shared_ptr<Window>& window,
      const SwapchainSupportedProperties& supportedProperties
   );
   ~Swapchain();
   void createFramebuffers(
         const RenderPass& renderPass,
         const DepthBuffer& depthBuffer,
         const MSAA& msaa
   );
   void presentImage(
         const uint32_t imageIndex,
         const std::vector<VkSemaphore> signalSemaphores,
         const VkQueue& presentQueue
   );
   void destroy();
   const uint32_t getNextImageIndex(const VkSemaphore& semaphore) const;
   const VkExtent2D& getExtent() const;
   const VkFormat& getImageFormat() const;
   const VkFramebuffer& getFramebuffer(const uint32_t imageIndex) const;
   const VkSwapchainKHR& get() const;
   const uint32_t getImageCount() const;
   const uint32_t getMinImageCount() const;
   const VkImageView& getImageView(const uint32_t index) const;

   // Used in isPhysicalDeviceSuitable function.
   bool isSwapchainAdequated(
      const VkPhysicalDevice& physicalDevice,
      const VkSurfaceKHR& surface
   );


private:

   void chooseBestSettings(
      const std::shared_ptr<Window>& window,
      const SwapchainSupportedProperties& supportedProperties,
      VkSurfaceFormatKHR& surfaceFormat,
      VkPresentModeKHR& presentMode,
      VkExtent2D& extent
   );

   void createAllImageViews();
   
   VkSurfaceFormatKHR chooseBestSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats
   );
    

   VkPresentModeKHR chooseBestPresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes
   );

   VkExtent2D chooseBestExtent(
         const VkSurfaceCapabilitiesKHR& capabilities,
         const std::shared_ptr<Window>& window
   );


   const bool existsMaxNumberOfSupportedImages(
         const VkSurfaceCapabilitiesKHR& capabilities
   ) const;

   VkDevice                   m_logicalDevice;

   VkSwapchainKHR             m_swapchain;
   VkFormat                   m_imageFormat;
   VkExtent2D                 m_extent;
   std::vector<VkImage>       m_images;
   std::vector<VkImageView>   m_imageViews;
   std::vector<VkFramebuffer> m_framebuffers;

   // Used for the creation of the Imgui instance.
   uint32_t                   m_minImageCount;
};
