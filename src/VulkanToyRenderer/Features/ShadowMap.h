#pragma once

#include <memory>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <VulkanToyRenderer/Descriptor/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptor/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Descriptor/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptor/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Pipeline/Graphics.h>
#include <VulkanToyRenderer/Model/Mesh.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>

template<typename T>
class ShadowMap
{

public:

   ShadowMap(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkExtent2D& extent,
         const uint32_t imagesCount,
         const VkFormat& format,
         const uint32_t& uboCount,
         const std::vector<Mesh<T>>* meshes,
         const std::vector<size_t>& modelIndices
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
   void bindData(
         const VkCommandBuffer& commandBuffer,
         const uint32_t currentFrame
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
   const std::shared_ptr<CommandPool>& getCommandPool() const;
   const Graphics& getGraphicsPipeline() const;
   const RenderPass& getRenderPass() const;

private:

   void createUBO(
         const VkPhysicalDevice& physicalDevice,
         const uint32_t& uboCount
   );
   void createDescriptorPool();
   void createDescriptorSets();
   void createGraphicsPipeline(const VkExtent2D& extent);
   void createRenderPass(const VkFormat& depthBufferFormat);
   void createFramebuffer(const uint32_t& imagesCount);

   VkDevice                         m_logicalDevice;

   uint32_t                         m_width;
   uint32_t                         m_height;

   Image                            m_image;
   std::shared_ptr<UBO>             m_ubo;

   RenderPass                       m_renderPass;

   DescriptorPool                   m_descriptorPool;
   DescriptorSets                   m_descriptorSets;

   std::shared_ptr<CommandPool>     m_commandPool;

   std::vector<VkFramebuffer>       m_framebuffers;

   Graphics                         m_graphicsPipeline;

   DescriptorTypes::UniformBufferObject::ShadowMap m_basicInfo;
   const std::vector<Mesh<T>>* m_opMeshes;
   const std::vector<size_t> m_modelIndices;
};
