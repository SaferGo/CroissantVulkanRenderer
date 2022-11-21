#pragma once

#include <memory>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>

class ShadowMap
{

public:

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
   void destroy();
   void updateUBO(
         const glm::mat4 modelM,
         const glm::fvec4 directionalLightStartPos,
         const glm::fvec4 directionalLightEndPos,
         const float aspect,
         const float zNear,
         const float zFar,
         const uint32_t& currentFrame
   );
   void createFramebuffer(
         const VkRenderPass& renderPass,
         const uint32_t& imagesCount
   );
   void createCommandPool(
         const VkCommandPoolCreateFlags& flags,
         const uint32_t& graphicsFamilyIndex
   );
   void allocCommandBuffers(const uint32_t& commandBuffersCount);
   const VkImageView& getShadowMapView() const;
   const VkSampler& getSampler() const;
   const glm::mat4& getLightSpace() const;
   const VkDescriptorSet& getDescriptorSet(const uint32_t index) const;
   const VkFramebuffer& getFramebuffer(const uint32_t imageIndex) const;
   const VkCommandBuffer& getCommandBuffer(const uint32_t index) const;
   const std::shared_ptr<CommandPool>& getCommandPool();

private:

   void createUBO(
         const VkPhysicalDevice& physicalDevice,
         const uint32_t& uboCount
   );
   void createDescriptorPool();
   void createDescriptorSets(
         const VkDescriptorSetLayout& descriptorSetLayout
   );

   VkDevice                         m_logicalDevice;

   uint32_t                         m_width;
   uint32_t                         m_height;

   Image                            m_image;
   std::shared_ptr<UBO>             m_ubo;

   DescriptorSets                   m_descriptorSets;
   DescriptorPool                   m_descriptorPool;

   std::shared_ptr<CommandPool>     m_commandPool;

   std::vector<VkFramebuffer>       m_framebuffers;

   DescriptorTypes::UniformBufferObject::ShadowMap m_basicInfo;

};
