#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Pipeline/Compute.h>
#include <VulkanToyRenderer/Descriptor/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptor/DescriptorSets.h>
#include <VulkanToyRenderer/Queue/QueueFamilyIndices.h>

class Computation
{

public:

   Computation();
   Computation(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const std::string& shaderName,
      const uint32_t& inSize,
      const uint32_t& outSize,
      const QueueFamilyIndices& queueFamilyIndices,
      DescriptorPool& descriptorPool,
      const std::vector<DescriptorInfo>& bufferInfos
   );
   ~Computation();
   void execute(const VkCommandBuffer& commandBuffer);
   void downloadData(
         const uint32_t offset,
         void* data,
         const uint32_t size
   );
   const VkBuffer& getOutBuffer() const;
   void destroy();

private:

   VkDevice                m_logicalDevice;
   Compute                 m_pipeline;
   DescriptorSets          m_descriptorSet;

   VkBuffer       m_inBuffer;
   VkBuffer       m_outBuffer;
   VkDeviceMemory m_inMemory;
   VkDeviceMemory m_outMemory;
};
