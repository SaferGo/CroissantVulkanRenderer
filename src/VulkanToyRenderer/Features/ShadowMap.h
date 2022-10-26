#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>

class ShadowMap
{

public:

   ShadowMap();
   ShadowMap(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t swapchainWidth,
         const uint32_t swapchainHeight,
         const VkFormat& depthBufferFormat,
         const VkSampleCountFlagBits& samplesCount,
         const VkDescriptorSetLayout& descriptorSetLayout
   );
   ~ShadowMap();
   void destroy();

private:

   void createImage(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t swapchainWidth,
         const uint32_t swapchainHeight,
         const VkFormat& depthBufferFormat,
         const VkSampleCountFlagBits& samplesCount
   );
   void createUBO(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice
   );
   void createDescriptorPool(const VkDevice& logicalDevice);
   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout
   );

   VkImage               m_image;
   VkDeviceMemory        m_imageMemory;
   VkImageView           m_imageView;
   Sampler               m_sampler;
   UBO                   m_ubo;
   DescriptorSets        m_descriptorSets;
   DescriptorPool        m_descriptorPool;

};
