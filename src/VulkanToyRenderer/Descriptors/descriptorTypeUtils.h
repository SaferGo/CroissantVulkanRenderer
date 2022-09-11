#pragma once

#include <vulkan/vulkan.h>

namespace descriptorTypeUtils
{

//////////////////////////////Uniform Buffer Object////////////////////////////

   void createUboPoolSize(
         const size_t size,
         VkDescriptorPoolSize& poolSize
   );
   void createDescriptorBufferInfo(
      const VkBuffer& uniformBuffer,
      VkDescriptorBufferInfo& bufferInfo
   );
   void createUboLayoutBinding(VkDescriptorSetLayoutBinding& layout); 
   void createUboWriteInfo(
         const VkDescriptorBufferInfo& bufferInfo,
         const VkDescriptorSet& descriptorSet,
         VkWriteDescriptorSet& descriptorWrite
   );

///////////////////////////////Combined Image Sampler//////////////////////////

   void createSamplerPoolSize(
         const size_t size,
         VkDescriptorPoolSize& poolSize
   );
   void createSamplerLayoutBinding(VkDescriptorSetLayoutBinding& layout);
   void createDescriptorImageInfo(
      const VkImageView& textureImageView,
      const VkSampler& textureSampler,
      VkDescriptorImageInfo& imageInfo
   );
   void createSamplerWriteInfo(
         const VkDescriptorImageInfo& imageInfo,
         const VkDescriptorSet& descriptorSet,
         VkWriteDescriptorSet& descriptorWrite
   );
};
