#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Queue/QueueFamilyIndices.h>

namespace bufferManager
{

   void createBuffer(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkDeviceSize size,
         const VkBufferUsageFlags usage,
         const VkMemoryPropertyFlags memoryProperties,
         VkDeviceMemory& memory,
         VkBuffer& buffer
   );
   void createSharedConcurrentBuffer(
            const VkPhysicalDevice& physicalDevice,
            const VkDevice& logicalDevice,
            const VkDeviceSize size,
            const VkBufferUsageFlags usage,
            const VkMemoryPropertyFlags memoryProperties,
            const QueueFamilyIndices& queueFamilyIndices,
            VkDeviceMemory& memory,
            VkBuffer& buffer
   );
   template<typename T>
   void createBufferAndTransferToDevice(
         const std::shared_ptr<CommandPool>& commandPool,
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         T* data,
         size_t size,
         VkQueue& graphicsQueue,
         const VkBufferUsageFlags usageDstBuffer,
         VkDeviceMemory& memory,
         VkBuffer& buffer
   );
   void freeMemory(
         const VkDevice& logicalDevice,
         VkDeviceMemory& memory
   );
   void destroyBuffer(
         const VkDevice& logicalDevice,
         VkBuffer& buffer
   );
   void copyBuffer(
      const std::shared_ptr<CommandPool>& commandPool,
      const VkDeviceSize size,
      VkBuffer& srcBuffer,
      VkBuffer& dstBuffer,
      VkQueue& graphicsQueue
   );
   template<typename T>
   void fillBuffer(
         const VkDevice& logicalDevice,
         T* data,
         const VkDeviceSize offset,
         const VkDeviceSize size,
         VkDeviceMemory& memory
   );
   void downloadDataFromBuffer(
         const VkDevice& logicalDevice,
         const VkDeviceSize& offset,
         const VkDeviceSize& size,
         const VkDeviceMemory& memory,
         void* outData
   );
   void allocBuffer(
      const VkDevice& logicalDevice,
      const VkPhysicalDevice& physicalDevice,
      const VkMemoryPropertyFlags memoryProperties,
      VkBuffer& buffer,
      VkDeviceMemory& memory
   );
   void bindBufferWithMemory(
      const VkDevice& logicalDevice,
      VkBuffer& buffer,
      VkDeviceMemory& memory
   );
};
