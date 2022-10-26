#include <VulkanToyRenderer/Features/ShadowMap.h>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Images/imageManager.h>

ShadowMap::ShadowMap() {}

ShadowMap::ShadowMap(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t swapchainWidth,
      const uint32_t swapchainHeight,
      const VkFormat& depthBufferFormat,
      const VkSampleCountFlagBits& samplesCount,
      const VkDescriptorSetLayout& descriptorSetLayout
) {
   createImage(
         physicalDevice,
         logicalDevice,
         swapchainWidth,
         swapchainHeight,
         depthBufferFormat,
         samplesCount
   );
   createDescriptorPool(logicalDevice);
   createUBO(physicalDevice, logicalDevice);
   createDescriptorSets(logicalDevice, descriptorSetLayout);
}

ShadowMap::~ShadowMap() {}

void ShadowMap::createDescriptorPool(const VkDevice& logicalDevice)
{
   m_descriptorPool = DescriptorPool(
         logicalDevice,
         {
            {
               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               15 * GRAPHICS_PIPELINE::SHADOWMAP::UBOS_COUNT // adsadsa
            },
            {
               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               15 * GRAPHICS_PIPELINE::SHADOWMAP::SAMPLERS_COUNT // adsadsad
            }
         },
         15 // adsadsad
   );
}

void ShadowMap::createImage(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t swapchainWidth,
      const uint32_t swapchainHeight,
      const VkFormat& depthBufferFormat,
      const VkSampleCountFlagBits& samplesCount
) {
   imageManager::createImage(
         physicalDevice,
         logicalDevice,
         swapchainWidth,
         swapchainHeight,
         depthBufferFormat,
         VK_IMAGE_TILING_OPTIMAL,
         (
            VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
         ),
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         false,
         1,
         samplesCount,
         m_image,
         m_imageMemory
   );
   imageManager::createImageView(
         logicalDevice,
         depthBufferFormat,
         m_image,
         VK_IMAGE_ASPECT_DEPTH_BIT,
         false,
         1,
         m_imageView
   );

   m_sampler = Sampler(
         physicalDevice,
         logicalDevice,
         1,
         VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
   );
}

void ShadowMap::createUBO(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice
) {
   m_ubo.createUniformBuffers(
         physicalDevice,
         logicalDevice,
         1,
         sizeof(DescriptorTypes::UniformBufferObject::ShadowMap)
   );
}

void ShadowMap::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout
) {
   std::vector<VkImageView> imageViews = {m_imageView};
   std::vector<Sampler> imageSamplers = {m_sampler};
   std::vector<UBO*> opUBOs = {&m_ubo};

   m_descriptorSets = DescriptorSets(
         logicalDevice,
         GRAPHICS_PIPELINE::SHADOWMAP::UBOS_INFO,
         GRAPHICS_PIPELINE::SHADOWMAP::SAMPLERS_INFO,
         imageViews,
         imageSamplers,
         opUBOs,
         descriptorSetLayout,
         m_descriptorPool
   );
}
