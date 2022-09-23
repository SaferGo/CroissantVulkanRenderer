#include <VulkanToyRenderer/Descriptors/DescriptorTypes/UBO.h>

#include <vector>

#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/DescriptorTypes.h>

UBO::UBO() {}

UBO::~UBO() {}

void UBO::createUniformBuffers(
      const VkPhysicalDevice physicalDevice,
      const VkDevice logicalDevice,
      const uint32_t nSets
) {
   VkDeviceSize size = sizeof(DescriptorTypes::UniformBufferObject);

   m_uniformBuffers.resize(nSets);
   m_uniformBufferMemories.resize(nSets);

   for (size_t i = 0; i < nSets; i++)
   {
      bufferManager::createBuffer(
            physicalDevice,
            logicalDevice,
            size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_uniformBufferMemories[i],
            m_uniformBuffers[i]
      );
   }
}

std::vector<VkDeviceMemory>& UBO::getUniformBufferMemories()
{
   return m_uniformBufferMemories;
}

VkDeviceMemory& UBO::getUniformBufferMemory(const uint32_t index)
{
   return m_uniformBufferMemories[index];
}

std::vector<VkBuffer>& UBO::getUniformBuffers()
{
   return m_uniformBuffers;
}

void UBO::destroyUniformBuffersAndMemories(const VkDevice& logicalDevice)
{
   for (size_t i = 0; i < m_uniformBuffers.size(); i++)
   {
      vkDestroyBuffer(
            logicalDevice,
            m_uniformBuffers[i],
            nullptr
      );
      vkFreeMemory(
            logicalDevice,
            m_uniformBufferMemories[i],
            nullptr
      );
   }
}
