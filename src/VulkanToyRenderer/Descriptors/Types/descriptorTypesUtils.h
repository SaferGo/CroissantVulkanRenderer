#pragma once

#include <vulkan/vulkan.h>

namespace descriptorTypesUtils{

   void createDescriptorBufferInfo(
      const VkBuffer& buffer,
      VkDescriptorBufferInfo& bufferInfo
   );
   void createDescriptorImageInfo(
      const VkImageView& imageView,
      const VkSampler& sampler,
      VkDescriptorImageInfo& imageInfo
   );

};
