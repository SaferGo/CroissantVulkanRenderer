#pragma once

#include <vulkan/vulkan.h>

class Sampler
{

public:

   Sampler();
   Sampler(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t mipLevels
   );
   ~Sampler();
   
   const VkSampler& getSampler() const;
   void destroySampler(const VkDevice& logicalDevice);

private:

      VkSampler     m_sampler;

};
