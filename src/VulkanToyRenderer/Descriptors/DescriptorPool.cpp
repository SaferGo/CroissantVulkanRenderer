#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>

#include <chrono>
#include <cstring>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VulkanToyRenderer/Descriptors/UniformBufferObject.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Settings/config.h>

DescriptorPool::DescriptorPool() {}
DescriptorPool::~DescriptorPool() {}

void DescriptorPool::createDescriptorPool(
      const VkDevice& logicalDevice,
      const size_t size,
      const size_t maxSets
) {
   VkDescriptorPoolSize poolSize{};
   poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSize.descriptorCount = static_cast<uint32_t>(size);

   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = 1;
   poolInfo.pPoolSizes = &poolSize;
   // Specifies the maximum number of descriptor sets that may be allocated.
   poolInfo.maxSets = static_cast<uint32_t>(maxSets);
   
   auto status = vkCreateDescriptorPool(
         logicalDevice,
         &poolInfo,
         nullptr,
         &m_descriptorPool
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor pool!");
}

void DescriptorPool::createUniformBuffers(
      const VkPhysicalDevice physicalDevice,
      const VkDevice logicalDevice,
      const size_t nSets
) {
   VkDeviceSize size = sizeof(UniformBufferObject);

   m_uniformBuffers.resize(nSets);
   m_uniformBuffersMemory.resize(nSets);

   for (size_t i = 0; i < nSets; i++)
   {
      bufferManager::createBuffer(
            physicalDevice,
            logicalDevice,
            size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_uniformBuffersMemory[i],
            m_uniformBuffers[i]
      );
   }
}

void DescriptorPool::createUboLayoutBinding(
      VkDescriptorSetLayoutBinding& uboLayoutBinding
) {
   // Binding used in the shader.
   uboLayoutBinding.binding = 0;
   uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   uboLayoutBinding.descriptorCount = 1;
   uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
   // Optional
   uboLayoutBinding.pImmutableSamplers = nullptr;
}

/*
 * Describes the details about every descriptor binding used in the shaders.
 */
void DescriptorPool::createDescriptorSetLayout(const VkDevice& logicalDevice)
{
   // Descriptor set layouts:

   VkDescriptorSetLayoutBinding uboLayoutBinding{};
   createUboLayoutBinding(uboLayoutBinding);

   // All the descriptors set layouts are combined into a single
   // VkDescriptorSetLayout object.
   VkDescriptorSetLayoutCreateInfo layoutInfo{};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = 1;
   layoutInfo.pBindings = &uboLayoutBinding;

   auto status = vkCreateDescriptorSetLayout(
         logicalDevice,
         &layoutInfo,
         nullptr,
         &m_descriptorSetLayout
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor set layout!");
}

void DescriptorPool::allocDescriptorSets(const VkDevice& logicalDevice)
{
   // Improve this. Make it more custom.
   std::vector<VkDescriptorSetLayout> layouts(
         config::MAX_FRAMES_IN_FLIGHT,
         m_descriptorSetLayout
   );

   m_descriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);

   VkDescriptorSetAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = m_descriptorPool;
   allocInfo.descriptorSetCount = static_cast<uint32_t>(
         m_descriptorSets.size()
   );
   allocInfo.pSetLayouts = layouts.data();


   auto status = vkAllocateDescriptorSets(
         logicalDevice,
         &allocInfo,
         m_descriptorSets.data()
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate descriptr sets!");
}

/*
 * Creates, allocates and configures the descriptor sets. It creates a 
 * descriptor set for each frame in flight.
 */
void DescriptorPool::createDescriptorSets(const VkDevice logicalDevice)
{
   allocDescriptorSets(logicalDevice);

   // Configures the descriptor sets
   for (size_t i = 0; i < m_descriptorSets.size(); i++)
   {
      // Specifies the buffer and the region within it that contains the data
      // for the descriptor set.
      VkDescriptorBufferInfo bufferInfo{};
      bufferInfo.buffer = m_uniformBuffers[i];
      bufferInfo.offset = 0;
      bufferInfo.range = VK_WHOLE_SIZE;

      // Describes how to update the descriptor(
      // (how and which buffer use to bind with the descriptor(the uniform) in
      // the shader).
      VkWriteDescriptorSet descriptorWrite{};
      descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorWrite.dstSet = m_descriptorSets[i];
      descriptorWrite.dstBinding = 0;
      descriptorWrite.dstArrayElement = 0;
      descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      descriptorWrite.descriptorCount = 1;
      descriptorWrite.pBufferInfo = &bufferInfo;
      // Optional
      // (For descriptors that refers to image data)
      descriptorWrite.pImageInfo = nullptr;
      // Optional
      // (For descriptors that refer to buffer views)
      descriptorWrite.pTexelBufferView = nullptr;

      vkUpdateDescriptorSets(
            logicalDevice,
            1,
            &descriptorWrite,
            0,
            nullptr
      );
   }
}

void DescriptorPool::destroyDescriptorPool(const VkDevice& logicalDevice)
{
   vkDestroyDescriptorPool(logicalDevice, m_descriptorPool, nullptr);
}

void DescriptorPool::destroyDescriptorSetLayout(const VkDevice& logicalDevice)
{
   vkDestroyDescriptorSetLayout(logicalDevice, m_descriptorSetLayout, nullptr);
}

void DescriptorPool::destroyUniformBuffersAndMemories(
      const VkDevice& logicalDevice
) {

   for (size_t i = 0; i < m_uniformBuffers.size(); i++)
   {
      vkDestroyBuffer(
            logicalDevice,
            m_uniformBuffers[i],
            nullptr
      );
      vkFreeMemory(
            logicalDevice,
            m_uniformBuffersMemory[i],
            nullptr
      );
   }

}

const std::vector<VkDescriptorSet> DescriptorPool::getDescriptorSets() const
{
   return m_descriptorSets;
}

const VkDescriptorSetLayout DescriptorPool::getDescriptorSetLayout() const
{
   return m_descriptorSetLayout;
}

void DescriptorPool::updateUniformBuffer(
         const VkDevice& logicalDevice,
         const uint8_t currentFrame,
         const VkExtent2D extent
) {
   static auto startTime = std::chrono::high_resolution_clock::now();

   auto currentTime = std::chrono::high_resolution_clock::now();
   float time = std::chrono::duration<float, std::chrono::seconds::period>(
         currentTime - startTime
   ).count();

   UniformBufferObject ubo{};
   ubo.model = glm::rotate(
         glm::mat4(1.0f),
         time * glm::radians(90.0f),
         glm::vec3(0.0f, 0.0f, 1.0f)
   );
   ubo.view = glm::lookAt(
         // Eye position
         glm::vec3(2.0, 2.0f, 2.0f),
         // Center position
         glm::vec3(0.0f, 0.0f, 0.0f),
         // Up Axis
         glm::vec3(0.0, 0.0f, 1.0f)
   );
   ubo.proj = glm::perspective(
         // VFOV
         glm::radians(45.0f),
         // Aspect
         (
          extent.width /
          (float)extent.height
         ),
         // Near plane
         0.1f,
         // Far plane
         10.0f
   );

   // GLM was designed for OpenGl, where the Y coordinate of the clip coord. is
   // inverted. To compensate for that, we have to flip the sign on the scaling
   // factor of the Y axis.
   ubo.proj[1][1] *= -1;

   void* data;
   vkMapMemory(
         logicalDevice,
         m_uniformBuffersMemory[currentFrame],
         0,
         sizeof(ubo),
         0,
         &data
   );
      memcpy(data, &ubo, sizeof(ubo));
   vkUnmapMemory(
         logicalDevice,
         m_uniformBuffersMemory[currentFrame]
   );
}
