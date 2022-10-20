#pragma once

#include <vulkan/vulkan.h>

class Sampler
{

public:

   Sampler();
   ~Sampler();

   void createSampler(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice
   );
   
   const VkSampler& getSampler() const;
   void destroySampler(const VkDevice& logicalDevice);

private:

      VkSampler     m_sampler;

};
