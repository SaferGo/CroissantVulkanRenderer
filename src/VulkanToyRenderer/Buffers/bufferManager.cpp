#include <VulkanToyRenderer/Buffers/bufferManager.h>

#include <stdexcept>
#include <cstring>

#include <stb/stb_image.h>
#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Model/Attributes.h>
#include <VulkanToyRenderer/Buffers/bufferUtils.h>
#include <VulkanToyRenderer/Commands/CommandPool.h>
#include <VulkanToyRenderer/Commands/commandUtils.h>

/*
 * It does:
 *    -Creates the memory for the buffer.
 *    -Allocates the buffer.
 *    -Binds the buffer with the memory.
 */
void bufferManager::createBuffer(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkDeviceSize size,
         const VkBufferUsageFlags usage,
         const VkMemoryPropertyFlags memoryProperties,
         VkDeviceMemory& memory,
         VkBuffer& buffer
) {
   VkBufferCreateInfo bufferInfo{};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size = size;
   // Indicates for which purposeS the data in the buffer is going to be
   // used.
   bufferInfo.usage = usage;
   // Indicates if the buffer can be owned by a specific queue family or
   // be shared between multiple at the same time.
   // (In this case we'll only use the graphics queue)
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   auto status = vkCreateBuffer(
         logicalDevice,
         &bufferInfo,
         nullptr,
         &buffer
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create buffer of usage: " + usage);

   allocBuffer(
         logicalDevice,
         physicalDevice,
         memoryProperties,
         buffer,
         memory
   );

   bindBufferWithMemory(
         logicalDevice,
         buffer,
         memory
   );
}

void bufferManager::bindBufferWithMemory(
      const VkDevice& logicalDevice,
      VkBuffer& buffer,
      VkDeviceMemory& memory
) {
   // 4 param. -> offset.
   vkBindBufferMemory(logicalDevice, buffer, memory, 0);
}


/*
 * Creates a cmd to copy the buffer to the buffer in the gpu making a cmd of
 * memory allocation.
 */
void bufferManager::copyBuffer(
      CommandPool& commandPool,
      const VkDeviceSize size,
      VkBuffer& srcBuffer,
      VkBuffer& dstBuffer,
      VkQueue& graphicsQueue
) {
   VkCommandBuffer commandBuffer;

   commandPool.allocCommandBuffer(commandBuffer, true);

   commandPool.beginCommandBuffer(
         VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
         commandBuffer
   );

      VkBufferCopy copyRegion{};
      // optional
      copyRegion.srcOffset = 0;
      // optional
      copyRegion.dstOffset = 0;
      copyRegion.size = size;

      commandUtils::ACTION::copyBufferToBuffer(
            srcBuffer,
            dstBuffer,
            1,
            copyRegion,
            commandBuffer
      );

   commandPool.endCommandBuffer(commandBuffer);

   commandPool.submitCommandBuffer(graphicsQueue, commandBuffer);
}

void bufferManager::allocBuffer(
      const VkDevice& logicalDevice,
      const VkPhysicalDevice& physicalDevice,
      const VkMemoryPropertyFlags memoryProperties,
      VkBuffer& buffer,
      VkDeviceMemory& memory
) {
   // -Memory requirements

   // VkMemoryRequirements struct:
   //    -Param. 1 -> The size of the required amount of memory in bytes.
   //    -Param. 2 -> The offset in bytes where the buffer begins in the
   //                 allocated region of memory, depends on bufferInfo
   //                 (usage and flags).
   //    -Param. 3 -> Bit field of the memory types that are suitable for the
   //                 buffer.
   //
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements(
         logicalDevice,
         buffer,
         &memRequirements
   );


   VkMemoryAllocateInfo allocInfo{};

   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize =  memRequirements.size;
   allocInfo.memoryTypeIndex = (
         bufferUtils::findMemoryType(
            physicalDevice,
            memRequirements.memoryTypeBits,
            memoryProperties
         )
   );

   auto status = vkAllocateMemory(
         logicalDevice,
         &allocInfo,
         nullptr,
         &memory
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to allocate buffer memory!");

}


template<typename T>
void bufferManager::createBufferAndTransferToDevice(
         CommandPool& commandPool,
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         T* data,
         const size_t size,
         VkQueue& graphicsQueue,
         const VkBufferUsageFlags usageDstBuffer,
         VkDeviceMemory& memory,
         VkBuffer& buffer
) {
   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   // Creates the staging buffer.
   createBuffer(
         physicalDevice,
         logicalDevice,
         size,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         stagingBufferMemory,
         stagingBuffer
   );

   fillBuffer(
         logicalDevice,
         data,
         0,
         size,
         stagingBufferMemory
   );

   // Creates the vertex buffer in the device(gpu).
   createBuffer(
         physicalDevice,
         logicalDevice,
         size,
         VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageDstBuffer,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         memory,
         buffer
   );

   copyBuffer(
         commandPool,
         size,
         stagingBuffer,
         buffer,
         graphicsQueue
   );

   destroyBuffer(logicalDevice, stagingBuffer);
   freeMemory(logicalDevice, stagingBufferMemory);
}



//////////////////////////////////Instances////////////////////////////////////

template void bufferManager::createBufferAndTransferToDevice<uint32_t>(
         CommandPool& commandPool,
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         uint32_t* data,
         const size_t size,
         VkQueue& graphicsQueue,
         const VkBufferUsageFlags usageDstBuffer,
         VkDeviceMemory& memory,
         VkBuffer& buffer
);

template void bufferManager::createBufferAndTransferToDevice<
   Attributes::PBR::Vertex
>(
         CommandPool& commandPool,
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         Attributes::PBR::Vertex* data,
         const size_t size,
         VkQueue& graphicsQueue,
         const VkBufferUsageFlags usageDstBuffer,
         VkDeviceMemory& memory,
         VkBuffer& buffer
);

template void bufferManager::createBufferAndTransferToDevice<
   Attributes::LIGHT::Vertex
>(
         CommandPool& commandPool,
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         Attributes::LIGHT::Vertex* data,
         const size_t size,
         VkQueue& graphicsQueue,
         const VkBufferUsageFlags usageDstBuffer,
         VkDeviceMemory& memory,
         VkBuffer& buffer
);


template void bufferManager::createBufferAndTransferToDevice<
   Attributes::SKYBOX::Vertex
>(
         CommandPool& commandPool,
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         Attributes::SKYBOX::Vertex* data,
         const size_t size,
         VkQueue& graphicsQueue,
         const VkBufferUsageFlags usageDstBuffer,
         VkDeviceMemory& memory,
         VkBuffer& buffer
);

///////////////////////////////////////////////////////////////////////////////

template<typename T>
void bufferManager::fillBuffer(
      const VkDevice& logicalDevice,
      T* data,
      const VkDeviceSize offset,
      const VkDeviceSize size,
      VkDeviceMemory& memory
) {
   void* memoryMap;
   // Allows us to access a region of the specified memory resource defined by
   // an offset and size.
   vkMapMemory(
         logicalDevice,
         memory,
         offset,
         size,
         0,
         &memoryMap
   );
      std::memcpy(memoryMap, data, size);
   vkUnmapMemory(logicalDevice, memory);
}
//////////////////////////////////Instances////////////////////////////////////
template void bufferManager::fillBuffer<Attributes::PBR::Vertex>(
      const VkDevice& logicalDevice,
      Attributes::PBR::Vertex* data,
      const VkDeviceSize offset,
      const VkDeviceSize size,
      VkDeviceMemory& memory
);
template void bufferManager::fillBuffer<Attributes::SKYBOX::Vertex>(
      const VkDevice& logicalDevice,
      Attributes::SKYBOX::Vertex* data,
      const VkDeviceSize offset,
      const VkDeviceSize size,
      VkDeviceMemory& memory
);
template void bufferManager::fillBuffer<uint32_t>(
      const VkDevice& logicalDevice,
      uint32_t* data,
      const VkDeviceSize offset,
      const VkDeviceSize size,
      VkDeviceMemory& memory
);
template void bufferManager::fillBuffer<stbi_uc>(
      const VkDevice& logicalDevice,
      stbi_uc* data,
      const VkDeviceSize offset,
      const VkDeviceSize size,
      VkDeviceMemory& memory
);
template void bufferManager::fillBuffer<Attributes::LIGHT::Vertex>(
      const VkDevice& logicalDevice,
      Attributes::LIGHT::Vertex* data,
      const VkDeviceSize offset,
      const VkDeviceSize size,
      VkDeviceMemory& memory
);
///////////////////////////////////////////////////////////////////////////////

void bufferManager::destroyBuffer(
      const VkDevice& logicalDevice,
      VkBuffer& buffer
) {
   vkDestroyBuffer(logicalDevice, buffer, nullptr);
}

void bufferManager::freeMemory(
      const VkDevice& logicalDevice,
      VkDeviceMemory& memory
) {
   vkFreeMemory(logicalDevice, memory, nullptr);
}
