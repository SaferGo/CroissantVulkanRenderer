#pragma once

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Image/Image.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>
#include <VulkanToyRenderer/Descriptor/DescriptorSets.h>
#include <VulkanToyRenderer/Pipeline/Graphics.h>
#include <VulkanToyRenderer/Command/CommandPool.h>

struct PushBlockPrefilterEnv
{
   glm::mat4 mvp;
   float roughness;
   int samplesCount = 32;
};

template<typename T>
class PrefilteredEnvMap
{

public:

   PrefilteredEnvMap(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool,
      const uint32_t dim,
      const std::vector<Mesh<T>>& meshes,
      const std::shared_ptr<Texture>& envMap
   );
   ~PrefilteredEnvMap();
   void destroy();
   const Image& get() const;

private:

   void createPipeline();
   void createOffscreenFramebuffer(
         const VkPhysicalDevice& physicalDevice,
         const VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool
   );
   void createRenderPass();
   void createTargetImage(const VkPhysicalDevice& physicalDevice);
   void createDescriptorPool();
   void createDescriptorSet(const std::shared_ptr<Texture>& envMap);
   void copyRegionOfImage(
         float face,
         float mipLevel,
         float viewportDim,
         const VkCommandBuffer& commandBuffer
   );
   void recordCommandBuffer(
         const std::shared_ptr<CommandPool>& commandPool,
         const VkQueue& graphicsQueue,
         const std::vector<Mesh<T>>& meshes
   );

   VkDevice                         m_logicalDevice;

   uint32_t                         m_dim;
   VkFormat                         m_format;
   uint32_t                         m_mipLevels;

   Image                            m_targetImage;
   Image                            m_offscreenImage;

   RenderPass                       m_renderPass;

   DescriptorSets                   m_descriptorSets;
   DescriptorPool                   m_descriptorPool;

   VkFramebuffer                    m_framebuffer;

   Graphics                         m_graphicsPipeline;

   PushBlockPrefilterEnv            m_pushBlock;
};
