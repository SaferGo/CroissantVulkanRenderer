#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>

UBO::UBO(
      const VkPhysicalDevice physicalDevice,
      const VkDevice logicalDevice,
      const uint32_t nSets,
      const size_t size
) : m_logicalDevice(logicalDevice)
{

   m_buffers.resize(nSets);
   m_memories.resize(nSets);

   for (size_t i = 0; i < nSets; i++)
   {
      bufferManager::createBuffer(
            physicalDevice,
            logicalDevice,
            (VkDeviceSize) size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_memories[i],
            m_buffers[i]
      );
   }
}

UBO::~UBO() {}

std::vector<VkDeviceMemory>& UBO::getMemories()
{
   return m_memories;
}

VkDeviceMemory& UBO::getMemory(const uint32_t index)
{
   return m_memories[index];
}

std::vector<VkBuffer>& UBO::get()
{
   return m_buffers;
}

VkBuffer& UBO::get(const size_t i)
{
   return m_buffers[i];
}

void UBO::destroy()
{
   for (size_t i = 0; i < m_buffers.size(); i++)
   {
      vkDestroyBuffer(
            m_logicalDevice,
            m_buffers[i],
            nullptr
      );
      vkFreeMemory(
            m_logicalDevice,
            m_memories[i],
            nullptr
      );
   }
}
