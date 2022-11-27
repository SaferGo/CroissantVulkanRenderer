#pragma once

#include <vulkan/vulkan.h>

class Sampler
{

public:

   Sampler(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t mipLevels,
      const VkSamplerAddressMode& addressMode,
      const VkFilter& filter
   );
   ~Sampler();
   const VkSampler& get() const;
   void destroy();

private:

   VkDevice      m_logicalDevice;

   VkSampler     m_sampler;
};
