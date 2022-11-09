#include <VulkanToyRenderer/Features/ShadowMap.h>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Images/imageManager.h>
#include <VulkanToyRenderer/Framebuffer/framebufferUtils.h>

ShadowMap::ShadowMap() {}

ShadowMap::ShadowMap(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t width,
      const uint32_t height,
      const VkFormat& format,
      const VkDescriptorSetLayout& descriptorSetLayout,
      const uint32_t& uboCount
) : m_width(width), m_height(height) {

   m_image = Image(
         physicalDevice,
         logicalDevice,
         width,
         height,
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

   createUBO(physicalDevice, logicalDevice, uboCount);
   createDescriptorPool(logicalDevice);
   createDescriptorSets(logicalDevice, descriptorSetLayout);
}

ShadowMap::~ShadowMap() {}

void ShadowMap::createCommandPool(
      const VkDevice& logicalDevice,
      const VkCommandPoolCreateFlags& flags,
      const uint32_t& graphicsFamilyIndex
) {
   m_commandPool = CommandPool(
         logicalDevice,
         flags,
         graphicsFamilyIndex
   );
}
void ShadowMap::updateUBO(
      const VkDevice& logicalDevice,
      const glm::mat4 modelM,
      const glm::fvec4 directionalLightStartPos,
      const glm::fvec4 directionalLightEndPos,
      const float aspect,
      const float zNear,
      const float zFar,
      const uint32_t& currentFrame
) {

   // TODO:
   // The model? and projection matrix don't need to be updated every frame.
   m_basicInfo.model = modelM;
   glm::mat4 proj = glm::ortho(
         -aspect,
         aspect,
         1.0f,
         -1.0f,
         0.0f,
         10.0f
   );
   float left = -aspect;
   float right = aspect;
   float top = -1.0f;
   float bottom = 1.0f;
   proj = glm::mat4(0.0f);
   proj[0][0] = 2.f / (right - left);
   proj[1][1] = 2.f / (bottom - top);
   proj[2][2] = 1.f / (0.0001f - zFar);
   proj[3][0] = -(right + left) / (right - left);
   proj[3][1] = -(bottom + top) / (bottom - top);
   proj[3][2] = 0.0001f / (0.0001f - zFar);
   proj[3][3] = 1.0f;
   //glm::mat4 proj = glm::perspective(
   //      45.0f,
   //      1.77865f,
   //      zNear,
   //      zFar
   //);
   proj[1][1] *= -1;

   glm::mat4 view = glm::lookAt(
         glm::fvec3(directionalLightStartPos),
         glm::fvec3(directionalLightEndPos),
         glm::fvec3(0.0f, 1.0f, 0.0f)
   );

   m_basicInfo.lightSpace = proj * view;

   size_t size = sizeof(m_basicInfo);
   UBOutils::updateUBO(logicalDevice, m_ubo, size, &m_basicInfo, currentFrame);
}

const VkCommandBuffer& ShadowMap::getCommandBuffer(const uint32_t index) const
{
   return m_commandPool.getCommandBuffer(index);
}

void ShadowMap::allocCommandBuffers(const uint32_t& commandBuffersCount)
{
   m_commandPool.allocCommandBuffers(commandBuffersCount);
}

void ShadowMap::createDescriptorPool(const VkDevice& logicalDevice)
{
   m_descriptorPool = DescriptorPool(
         logicalDevice,
         {
            {
               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
               15 * GRAPHICS_PIPELINE::SHADOWMAP::UBOS_COUNT // adsadsa
            }
         },
         15 // adsadsad
   );
}

void ShadowMap::createUBO(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t& uboCount
) {
   m_ubo.createUniformBuffers(
         physicalDevice,
         logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::ShadowMap)
   );
}

void ShadowMap::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout
) {
   std::vector<UBO*> opUBOs = {&m_ubo};

   m_descriptorSets = DescriptorSets(
         logicalDevice,
         GRAPHICS_PIPELINE::SHADOWMAP::UBOS_INFO,
         {},
         {},
         nullptr,
         nullptr,
         opUBOs,
         descriptorSetLayout,
         m_descriptorPool
   );
}

const VkImageView& ShadowMap::getShadowMapView() const
{
   return m_image.getImageView();
}

const VkSampler& ShadowMap::getSampler() const
{
   return m_image.getSampler();
}


const VkDescriptorSet& ShadowMap::getDescriptorSet(const uint32_t index) const
{
   return m_descriptorSets.get(index);
}

const VkFramebuffer& ShadowMap::getFramebuffer(const uint32_t imageIndex) const
{
   return m_framebuffers[imageIndex];
}

CommandPool& ShadowMap::getCommandPool()
{
   return m_commandPool;
}

void ShadowMap::createFramebuffer(
      const VkDevice& logicalDevice,
      const VkRenderPass& renderPass,
      const uint32_t& imagesCount
) {

   m_framebuffers.resize(imagesCount);

   // We'll write in the sampler to later use it in the scene fragment shader.
   std::vector<VkImageView> attachments = {m_image.getImageView()};

   for (uint32_t i = 0; i < imagesCount; i++)
   {
      framebufferUtils::createFramebuffer(
            logicalDevice,
            renderPass,
            attachments,
            m_width,
            m_height,
            1,
            m_framebuffers[i]
      );
   }
}

void ShadowMap::destroy(const VkDevice& logicalDevice)
{
   m_descriptorPool.destroyDescriptorPool(logicalDevice);
   m_image.destroy(logicalDevice);
   m_ubo.destroyUniformBuffersAndMemories(logicalDevice);
   m_commandPool.destroy();
   for (auto& framebuffer : m_framebuffers)
      vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
}
