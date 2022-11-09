#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Images/Image.h>

namespace renderTarget
{

   class DepthBuffer
   {
   
   public:
   
      DepthBuffer();
      DepthBuffer(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkExtent2D& swapchainExtent,
         const VkSampleCountFlagBits& samplesCount
      );
      ~DepthBuffer();
      const VkImageView& getImageView() const;
      const VkFormat& getFormat() const;

      void destroy(const VkDevice& logicalDevice);
   
   private:
   
      Image m_image;
      VkFormat m_format;
   
   };

   class MSAA
   {
   
   public:
   
      MSAA();
      MSAA(
            const VkPhysicalDevice& physicalDevice,
            const VkDevice& logicalDevice,
            const VkExtent2D& swapchainExtent,
            const VkFormat& swapchainFormat
      );
      ~MSAA();

      const VkSampleCountFlagBits& getSamplesCount() const;
      const VkImageView& getImageView() const;

      void destroy(
         const VkDevice& logicalDevice
      );
   
   private:
   
      Image                 m_image;
      VkSampleCountFlagBits m_samplesCount;
   
   };
};
