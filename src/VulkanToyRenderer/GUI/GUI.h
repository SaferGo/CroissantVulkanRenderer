#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <VulkanToyRenderer/Descriptor/DescriptorPool.h>
#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Swapchain/Swapchain.h>
#include <VulkanToyRenderer/Camera/Camera.h>
#include <VulkanToyRenderer/RenderPass/RenderPass.h>
#include <VulkanToyRenderer/Model/Model.h>

class GUI
{

public:

   GUI(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkInstance& vkInstance,
         const std::shared_ptr<Swapchain>& swapchain,
         const uint32_t& graphicsFamilyIndex,
         const VkQueue& graphicsQueue,
         const std::shared_ptr<Window>& window
   );
   ~GUI();
   void recordCommandBuffer(
         const uint8_t currentFrame,
         const uint8_t imageIndex,
         const std::vector<VkClearValue>& clearValues
   );
   void draw(
         const std::vector<std::shared_ptr<Model>>& models,
         const std::shared_ptr<Camera>& camera,
         const std::vector<size_t>& normalModelIndices,
         const std::vector<size_t>& lightModelIndices,
         const std::string& deviceName,
         const double mpf,
         const VkSampleCountFlagBits samplesCount,
         const uint32_t apiVersion
   );
   const VkCommandBuffer& getCommandBuffer(const uint32_t index) const;
   const bool isCursorPositionInGUI() const;
   void destroy();

private:

      
   void displayLightModels(
         std::vector<std::shared_ptr<Model>> models,
         const std::vector<size_t> indices
   );
   void displayCamera(const std::shared_ptr<Camera>& camera);
   void createModelsWindow(
         std::vector<std::shared_ptr<Model>> models,

         const std::vector<size_t> objectIndices,
         const std::vector<size_t> lightIndices,
         const std::shared_ptr<Camera>& camera
   );
   void createProfilingWindow(
         const std::string& deviceName,
         const double mpf,
         const VkSampleCountFlagBits samplesCount,
         const uint32_t apiVersion
   );
   void createSlider(
         const std::string& subMenuName, const std::string& sliceName,
         const float& maxV,
         const float& minV,
         float& value
   );
   void createTransformationsInfo(
         glm::vec4& pos,
         glm::vec3& rot,
         glm::vec3& size,
         const std::string& modelName
   );
   void createTranslationSliders(
         const std::string& name,
         const std::string& treeNodeName,
         glm::fvec4& pos,
         const float minR,
         const float maxR
   );
   void createRotationSliders(
         const std::string& name,
         glm::fvec3& pos,
         const float minR,
         const float maxR
   );
   void createSizeSliders(
         const std::string& name,
         glm::fvec3& pos,
         const float minR,
         const float maxR
   );
   void createRenderPass();
   void createFrameBuffers();
   void uploadFonts(const VkQueue& graphicsQueue);
   void applyStyle();
   void modelsWindow(
         std::vector<std::shared_ptr<Model>> models,
         const std::vector<size_t> normalModelIndices,
         const std::vector<size_t> lightModelIndices
   );
   void cameraWindow(glm::fvec3& cameraPos);

   VkDevice                         m_logicalDevice;

   std::vector<VkFramebuffer>       m_framebuffers;
   std::shared_ptr<CommandPool>     m_commandPool;
   DescriptorPool  m_descriptorPool;
   RenderPass                       m_renderPass;

   // Observer pointers
   const Swapchain*                 m_opSwapchain;

};
