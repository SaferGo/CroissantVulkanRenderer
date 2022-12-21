#include <CroissantRenderer/Features/ShadowMap.h>

#include <memory>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Settings/graphicsPipelineConfig.h>
#include <CroissantRenderer/Settings/config.h>
#include <CroissantRenderer/Descriptor/Types/UBO/UBO.h>
#include <CroissantRenderer/Descriptor/Types/Sampler/Sampler.h>
#include <CroissantRenderer/Descriptor/DescriptorSets.h>
#include <CroissantRenderer/Descriptor/DescriptorPool.h>
#include <CroissantRenderer/Descriptor/Types/DescriptorTypes.h>
#include <CroissantRenderer/Descriptor/Types/UBO/UBOutils.h>
#include <CroissantRenderer/Image/imageManager.h>
#include <CroissantRenderer/Framebuffer/framebufferManager.h>
#include <CroissantRenderer/Math/mathUtils.h>
#include <CroissantRenderer/Command/commandManager.h>
#include <CroissantRenderer/Model/Attributes.h>
#include <CroissantRenderer/RenderPass/attachmentUtils.h>

template<typename T>
ShadowMap<T>::ShadowMap(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkExtent2D& extent,
      const uint32_t imagesCount,
      const VkFormat& format,
      const uint32_t& uboCount,
      const std::vector<Mesh<T>>* meshes,
      const std::vector<size_t>& modelIndices
) : m_logicalDevice(logicalDevice), 
    m_width(extent.width),
    m_height(extent.height),
    m_opMeshes(meshes)
{

   m_image = Image(
         physicalDevice,
         logicalDevice,
         m_width,
         m_height,
         format,
         VK_IMAGE_TILING_OPTIMAL,
         (
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT
         ),
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         false,
         1,
         VK_SAMPLE_COUNT_1_BIT,
         VK_IMAGE_ASPECT_DEPTH_BIT,
         VK_COMPONENT_SWIZZLE_R,
         VK_COMPONENT_SWIZZLE_G,
         VK_COMPONENT_SWIZZLE_B,
         VK_COMPONENT_SWIZZLE_A,
         VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
         VK_FILTER_NEAREST
   );

   createUBO(physicalDevice, uboCount);
   createRenderPass(format);
   createFramebuffer(imagesCount);
   createGraphicsPipeline(extent);
   createDescriptorPool();
   createDescriptorSets();
}

template<typename T>
void ShadowMap<T>::createGraphicsPipeline(const VkExtent2D& extent)
{
   m_graphicsPipeline = Graphics(
         m_logicalDevice,
         GraphicsPipelineType::SHADOWMAP,
         extent,
         m_renderPass,
         {
            {
               shaderType::VERTEX,
               "shadowMap"
            }
         },
         VK_SAMPLE_COUNT_1_BIT,
         Attributes::PBR::getBindingDescription(),
         Attributes::PBR::getAttributeDescriptions(),
         m_modelIndices,
         GRAPHICS_PIPELINE::SHADOWMAP::UBOS_INFO,
         {},
         {}
   );
}

template<typename T>
ShadowMap<T>::~ShadowMap() {}

template<typename T>
void ShadowMap<T>::createCommandPool(
      const VkCommandPoolCreateFlags& flags,
      const uint32_t& graphicsFamilyIndex
) {
   m_commandPool = std::make_shared<CommandPool>(
         m_logicalDevice,
         flags,
         graphicsFamilyIndex
   );
}

template<typename T>
void ShadowMap<T>::updateUBO(
      const glm::mat4 modelM,
      const glm::fvec4 directionalLightStartPos,
      const glm::fvec4 directionalLightEndPos,
      const float aspect,
      const float zNear,
      const float zFar,
      const uint32_t& currentFrame
) {

   // TODO: Improve this.
   // The model and projection matrix don't need to be updated every frame.
   m_basicInfo.model = modelM;
   glm::mat4 proj = mathUtils::getUpdatedProjMatrix(
         glm::radians(config::FOV),
         aspect,
         zNear,
         zFar
   );

   glm::mat4 view = glm::lookAt(
         glm::fvec3(directionalLightStartPos),
         glm::fvec3(directionalLightEndPos),
         glm::fvec3(0.0f, 1.0f, 0.0f)
   );

   m_basicInfo.lightSpace = proj * view;

   size_t size = sizeof(m_basicInfo);
   UBOutils::updateUBO(
         m_logicalDevice,
         m_ubo,
         size,
         &m_basicInfo,
         currentFrame
   );
}

template<typename T>
const VkCommandBuffer& ShadowMap<T>::getCommandBuffer(const uint32_t index) const
{
   return m_commandPool->getCommandBuffer(index);
}

template<typename T>
void ShadowMap<T>::allocCommandBuffers(const uint32_t& commandBuffersCount)
{
   m_commandPool->allocCommandBuffers(commandBuffersCount);
}

template<typename T>
void ShadowMap<T>::createDescriptorPool()
{
   m_descriptorPool = DescriptorPool(
         m_logicalDevice,
         {
            {
               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               15 * GRAPHICS_PIPELINE::SHADOWMAP::UBOS_COUNT
            }
         },
         15 // TODO: Improve this.
   );
}

template<typename T>
void ShadowMap<T>::createUBO(
      const VkPhysicalDevice& physicalDevice,
      const uint32_t& uboCount
) {
   m_ubo = std::make_shared<UBO>(
         physicalDevice,
         m_logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::ShadowMap)
   );
}

template<typename T>
void ShadowMap<T>::createDescriptorSets()
{
   std::vector<UBO*> opUBOs = {m_ubo.get()};

   m_descriptorSets = DescriptorSets(
         m_logicalDevice,
         GRAPHICS_PIPELINE::SHADOWMAP::UBOS_INFO,
         {},
         {},
         m_graphicsPipeline.getDescriptorSetLayout(),
         m_descriptorPool,
         nullptr,
         opUBOs
   );
}

template<typename T>
const VkImageView& ShadowMap<T>::getShadowMapView() const
{
   return m_image.getImageView();
}

template<typename T>
const VkSampler& ShadowMap<T>::getSampler() const
{
   return m_image.getSampler();
}

template<typename T>
void ShadowMap<T>::bindData(
      const VkCommandBuffer& commandBuffer,
      const uint32_t currentFrame
) {
   for (auto mesh = m_opMeshes->begin(); mesh != m_opMeshes->end(); mesh++)
   {
      commandManager::state::bindVertexBuffers(
            {mesh->vertexBuffer},
            // Offsets.
            {0},
            // Index of first binding.
            0,
            // Bindings count.
            1,
            commandBuffer
      );
      commandManager::state::bindIndexBuffer(
            mesh->indexBuffer,
            // Offset.
            0,
            VK_INDEX_TYPE_UINT32,
            commandBuffer
      );

      commandManager::state::bindDescriptorSets(
            m_graphicsPipeline.getPipelineLayout(),
            PipelineType::GRAPHICS,
            // Index of first descriptor set.
            0,
            {getDescriptorSet(currentFrame)},
            // Dynamic offsets.
            {},
            commandBuffer
      );

      commandManager::action::drawIndexed(
            // Index Count
            mesh->indices.size(),
            // Instance Count
            1,
            // First index.
            0,
            // Vertex Offset.
            0,
            // First Intance.
            0,
            commandBuffer
      );
   }
}


template<typename T>
const VkDescriptorSet& ShadowMap<T>::getDescriptorSet(const uint32_t index) const
{
   return m_descriptorSets.get(index);
}

template<typename T>
const VkFramebuffer& ShadowMap<T>::getFramebuffer(const uint32_t imageIndex) const
{
   return m_framebuffers[imageIndex];
}

template<typename T>
const std::shared_ptr<CommandPool>& ShadowMap<T>::getCommandPool() const
{
   return m_commandPool;
}

template<typename T>
const glm::mat4& ShadowMap<T>::getLightSpace() const
{
   return m_basicInfo.lightSpace;
}

template<typename T>
const RenderPass& ShadowMap<T>::getRenderPass() const
{
   return m_renderPass;
}

template<typename T>
const Graphics& ShadowMap<T>::getGraphicsPipeline() const
{
   return m_graphicsPipeline;
}

template<typename T>
void ShadowMap<T>::createFramebuffer(const uint32_t& imagesCount)
{
   m_framebuffers.resize(imagesCount);

   // We'll write in the sampler to later use it in the scene fragment shader.
   std::vector<VkImageView> attachments = {m_image.getImageView()};

   for (uint32_t i = 0; i < imagesCount; i++)
   {
      framebufferManager::createFramebuffer(
            m_logicalDevice,
            m_renderPass.get(),
            attachments,
            m_width,
            m_height,
            1,
            m_framebuffers[i]
      );
   }
}

template<typename T>
void ShadowMap<T>::destroy()
{
   m_graphicsPipeline.destroy();
   m_descriptorPool.destroy();
   m_image.destroy();
   m_ubo->destroy();
   m_commandPool->destroy();

   m_renderPass.destroy();

   for (auto& framebuffer : m_framebuffers)
      vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
}

template<typename T>
void ShadowMap<T>::createRenderPass(const VkFormat& depthBufferFormat)
{
   // - Attachments

   VkAttachmentDescription shadowMapAttachment{};
   attachmentUtils::createAttachmentDescriptionWithStencil(
         depthBufferFormat,
         VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         shadowMapAttachment
   );


   // Attachment references
   
   VkAttachmentReference shadowMapAttachmentRef{};
   attachmentUtils::createAttachmentReference(
         0,
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
         shadowMapAttachmentRef
   );

   // Subpasses
   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.flags = 0;
   subpass.pDepthStencilAttachment = &shadowMapAttachmentRef;

   m_renderPass = RenderPass(
         m_logicalDevice,
         {shadowMapAttachment},
         {subpass},
         {}
         //dependencies
   );

}


////////////////////////////////////INSTANCES//////////////////////////////////
template class ShadowMap<Attributes::PBR::Vertex>;
