#pragma once

#include <vulkan/vulkan.h>

class Sampler
{

public:

   Sampler();
   Sampler(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t mipLevels,
      const VkSamplerAddressMode& addressMode
   );
   ~Sampler();
   
   const VkSampler& get() const;
   void destroySampler(const VkDevice& logicalDevice);

private:

      VkSampler     m_sampler;

};
