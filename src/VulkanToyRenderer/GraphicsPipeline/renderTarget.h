#pragma once

#include <vulkan/vulkan.h>

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
   
      VkImage m_image;
      VkDeviceMemory m_imageMemory;
      VkImageView m_imageView;
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
   
      VkImage        m_image;
      VkDeviceMemory m_imageMemory;
      VkImageView    m_imageView;
      VkSampleCountFlagBits m_samplesCount;
   
   };
};
