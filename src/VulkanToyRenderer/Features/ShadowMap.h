#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>

class ShadowMap
{

public:

   ShadowMap();
   ShadowMap(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t width,
         const uint32_t height,
         const VkFormat& format,
         const VkDescriptorSetLayout& descriptorSetLayout,
         const uint32_t& uboCount
   );
   ~ShadowMap();
   void destroy(const VkDevice& logicalDevice);
   void updateUBO(
         const VkDevice& logicalDevice,
         const glm::mat4 modelM,
         const glm::fvec4 directionalLightStartPos,
         const glm::fvec4 directionalLightEndPos,
         const float aspect,
         const float zNear,
         const float zFar,
         const uint32_t& currentFrame
   );
   void createFramebuffer(
         const VkDevice& logicalDevice,
         const VkRenderPass& renderPass,
         const uint32_t& imagesCount
   );
   void createCommandPool(
         const VkDevice& logicalDevice,
         const VkCommandPoolCreateFlags& flags,
         const uint32_t& graphicsFamilyIndex
   );
   void allocCommandBuffers(const uint32_t& commandBuffersCount);
   const VkImageView& getShadowMapView() const;
   const VkSampler& getSampler() const;
   const VkDescriptorSet& getDescriptorSet(const uint32_t index) const;
   const VkFramebuffer& getFramebuffer(const uint32_t imageIndex) const;
   const VkCommandBuffer& getCommandBuffer(const uint32_t index) const;
   CommandPool& getCommandPool();

private:

   void createUBO(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   );
   void createDescriptorPool(const VkDevice& logicalDevice);
   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout
   );

   uint32_t                   m_width;
   uint32_t                   m_height;
   Image                      m_image;
   UBO                        m_ubo;
   DescriptorSets             m_descriptorSets;
   DescriptorPool             m_descriptorPool;
   CommandPool                m_commandPool;
   std::vector<VkFramebuffer> m_framebuffers;

   DescriptorTypes::UniformBufferObject::ShadowMap m_basicInfo;

};
