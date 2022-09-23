#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Swapchain/Swapchain.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>
#include <VulkanToyRenderer/Model/Model.h>

class GUI
{

public:

   GUI(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkInstance& vkInstance,
         const Swapchain& swapchain,
         const uint32_t& graphicsFamilyIndex,
         const VkQueue& graphicsQueue,
         Window& window
   );
   ~GUI();
   void recordCommandBuffer(
         const uint8_t currentFrame,
         const uint8_t imageIndex,
         const std::vector<VkClearValue>& clearValues
   );
   void draw(std::vector<std::shared_ptr<Model>> models, glm::fvec3& cameraPos);
   const VkCommandBuffer& getCommandBuffer(const uint32_t index) const;
   void destroy(const VkDevice& logicalDevice);

private:
      
   void createRenderPass();
   void createFrameBuffers();
   void uploadFonts(const VkQueue& graphicsQueue);
   void applyStyle();
   void modelsWindow(std::vector<std::shared_ptr<Model>> models);
   void cameraWindow(glm::fvec3& cameraPos);

   std::vector<VkFramebuffer>   m_framebuffers;
   CommandPool                  m_commandPool;
   DescriptorPool               m_descriptorPool;
   RenderPass                   m_renderPass;

   // Observer pointers
   const Swapchain*             m_opSwapchain;
   const VkDevice*              m_opLogicalDevice;

};
