#pragma once

#include <vector>

#include <vulkan/vulkan.h>

class UBO
{

public:

   UBO(
         const VkPhysicalDevice physicalDevice,
         const VkDevice logicalDevice,
         const uint32_t nSets,
         const size_t size
   );
   ~UBO();
   std::vector<VkDeviceMemory>& getMemories();
   VkDeviceMemory& getMemory(const uint32_t index);
   std::vector<VkBuffer>& get();
   VkBuffer& get(const size_t i);
   void destroy();

private:

   VkDevice                     m_logicalDevice;

   std::vector<VkBuffer>        m_buffers;
   std::vector<VkDeviceMemory>  m_memories;

};
