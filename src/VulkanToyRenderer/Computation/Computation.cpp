#include <VulkanToyRenderer/Computation/Computation.h>

#include <string>

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Settings/computePipelineConfig.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptors/descriptorSetLayoutUtils.h>
#include <VulkanToyRenderer/Commands/commandManager.h>
#include <VulkanToyRenderer/BufferManager/bufferManager.h>

Computation::Computation() {}

Computation::Computation(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const std::string& shaderName,
      const uint32_t& inSize,
      const uint32_t& outSize,
      const QueueFamilyIndices& queueFamilyIndices,
      DescriptorPool& descriptorPool
) : m_logicalDevice(logicalDevice)
{

   bufferManager::createSharedConcurrentBuffer(
         physicalDevice,
         logicalDevice,
         inSize,
         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         (
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
         ),
         queueFamilyIndices,
         m_inMemory,
         m_inBuffer
   );
   bufferManager::createSharedConcurrentBuffer(
         physicalDevice,
         logicalDevice,
         outSize,
         VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         (
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
         ),
         queueFamilyIndices,
         m_outMemory,
         m_outBuffer
   );

   descriptorSetLayoutUtils::compute::createDescriptorSetLayout(
         m_logicalDevice,
         COMPUTE_PIPELINE::BRDF::BUFFERS_INFO,
         m_descriptorSetLayout
   );

   m_descriptorSet = DescriptorSets(
         m_logicalDevice,
         COMPUTE_PIPELINE::BRDF::BUFFERS_INFO,
         {
            m_inBuffer,
            m_outBuffer
         },
         m_descriptorSetLayout,
         descriptorPool
   );

   m_pipeline = Compute(
         m_logicalDevice,
         m_descriptorSetLayout,
         {
            shaderType::COMPUTE,
            "BRDF"
         }
   );

}

Computation::~Computation() {}

void Computation::execute(const VkCommandBuffer& commandBuffer)
{
   commandManager::state::bindPipeline(
         m_pipeline.get(),
         PipelineType::COMPUTE,
         commandBuffer
   );
   commandManager::state::bindDescriptorSets(
         m_pipeline.getPipelineLayout(),
         PipelineType::COMPUTE,
         // Index of first descriptor set.
         0,
         {m_descriptorSet.get(0)},
         // Dynamic offsets.
         {},
         commandBuffer
   );
   commandManager::action::dispatch(
         config::BRDF_WIDTH,
         config::BRDF_HEIGHT,
         1,
         commandBuffer
   );
}

void Computation::downloadData(
      const uint32_t offset,
      void* data,
      const uint32_t size
) {
   bufferManager::downloadDataFromBuffer(
         m_logicalDevice,
         offset,
         size,
         m_outMemory,
         data
   );
}

void Computation::destroy()
{
   vkDestroyBuffer(m_logicalDevice, m_inBuffer, nullptr);
   vkDestroyBuffer(m_logicalDevice, m_outBuffer, nullptr);
   vkFreeMemory(m_logicalDevice, m_inMemory, nullptr);
   vkFreeMemory(m_logicalDevice, m_outMemory, nullptr);

   descriptorSetLayoutUtils::destroyDescriptorSetLayout(
         m_logicalDevice,
         m_descriptorSetLayout
   );

   m_pipeline.destroy();
}
