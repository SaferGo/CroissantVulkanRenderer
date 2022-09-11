#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>

#include <chrono>
#include <array>
#include <cstring>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VulkanToyRenderer/Descriptors/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/descriptorTypeUtils.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Settings/config.h>

DescriptorPool::DescriptorPool() {}
DescriptorPool::~DescriptorPool() {}

void DescriptorPool::createDescriptorPool(
      const VkDevice& logicalDevice,
      const size_t size,
      const size_t maxSets
) {

   std::array<VkDescriptorPoolSize, 2> poolSizes{};
   descriptorTypeUtils::createUboPoolSize(size, poolSizes[0]);
   descriptorTypeUtils::createSamplerPoolSize(size, poolSizes[1]);

   VkDescriptorPoolCreateInfo poolInfo{};
   poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
   poolInfo.pPoolSizes = poolSizes.data();
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
   VkDeviceSize size = sizeof(DescriptorTypes::UniformBufferObject);

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


/*
 * Combines all the different types of descriptors set(layouts) in one
 * layout.
 */
void DescriptorPool::createDescriptorSetLayout(const VkDevice& logicalDevice)
{
   // Descriptor set layouts:

   // - UBO binding(layout)
   VkDescriptorSetLayoutBinding uboLayoutBinding{};
   descriptorTypeUtils::createUboLayoutBinding(uboLayoutBinding);

   // -Combined image sampler(Layout)
   VkDescriptorSetLayoutBinding samplerLayoutBinding{};
   descriptorTypeUtils::createSamplerLayoutBinding(samplerLayoutBinding);

   // All the descriptors set layouts are combined into a single
   // VkDescriptorSetLayout object.
   std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
      uboLayoutBinding,
      samplerLayoutBinding
   };
   VkDescriptorSetLayoutCreateInfo layoutInfo{};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
   layoutInfo.pBindings = bindings.data();

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
void DescriptorPool::createDescriptorSets(
      const VkDevice logicalDevice,
      const VkImageView& textureImageView,
      const VkSampler& textureSampler
) {
   allocDescriptorSets(logicalDevice);

   // Configures the descriptor sets
   for (size_t i = 0; i < m_descriptorSets.size(); i++)
   {

      VkDescriptorBufferInfo bufferInfo{};
      descriptorTypeUtils::createDescriptorBufferInfo(
            m_uniformBuffers[i],
            bufferInfo
      );
      VkDescriptorImageInfo imageInfo{};
      descriptorTypeUtils::createDescriptorImageInfo(
            textureImageView,
            textureSampler,
            imageInfo
      );

      // Describes how to update the descriptors.
      // (how and which buffer/image use to bind with the each descriptor)
      std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

      descriptorTypeUtils::createUboWriteInfo(
            bufferInfo,
            m_descriptorSets[i],
            descriptorWrites[0]
      );
      descriptorTypeUtils::createSamplerWriteInfo(
            imageInfo,
            m_descriptorSets[i],
            descriptorWrites[1]
      );

      vkUpdateDescriptorSets(
         logicalDevice,
         static_cast<uint32_t>(descriptorWrites.size()),
         descriptorWrites.data(),
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

   DescriptorTypes::UniformBufferObject ubo{};
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
