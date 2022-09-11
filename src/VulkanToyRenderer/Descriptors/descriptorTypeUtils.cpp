#include <VulkanToyRenderer/Descriptors/descriptorTypeUtils.h>

#include <vulkan/vulkan.h>

///////////////////////////////////Uniform Buffer Object///////////////////////
void descriptorTypeUtils::createUboPoolSize(
      const size_t size,
      VkDescriptorPoolSize& poolSize
) {
   poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSize.descriptorCount = static_cast<uint32_t>(size);
}

void descriptorTypeUtils::createUboLayoutBinding(
      VkDescriptorSetLayoutBinding& layout
) {
   // Binding used in the shader.
   layout.binding = 0;
   layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   layout.descriptorCount = 1;
   layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
   // Optional
   layout.pImmutableSamplers = nullptr;
}

void descriptorTypeUtils::createDescriptorBufferInfo(
      const VkBuffer& uniformBuffer,
      VkDescriptorBufferInfo& bufferInfo
) {
   // Specifies the buffer and the region within it that contains the data
   // for the descriptor set.
   bufferInfo.buffer = uniformBuffer;
   bufferInfo.offset = 0;
   bufferInfo.range = VK_WHOLE_SIZE;
}

void descriptorTypeUtils::createUboWriteInfo(
      const VkDescriptorBufferInfo& bufferInfo,
      const VkDescriptorSet& descriptorSet,
      VkWriteDescriptorSet& descriptorWrite
) {
   descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrite.dstSet = descriptorSet;
   descriptorWrite.dstBinding = 0;
   descriptorWrite.dstArrayElement = 0;
   descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   descriptorWrite.descriptorCount = 1;
   descriptorWrite.pBufferInfo = &bufferInfo;
   // Optional
   // (For descriptors that refers to image data)
   descriptorWrite.pImageInfo = nullptr;
   // Optional
   // (For descriptors that refer to buffer views)
   descriptorWrite.pTexelBufferView = nullptr;
}

//////////////////////////Combined Image Sampler///////////////////////////////

void descriptorTypeUtils::createSamplerPoolSize(
      const size_t size,
      VkDescriptorPoolSize& poolSize
) {
   poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   poolSize.descriptorCount = static_cast<uint32_t>(size);
}

void descriptorTypeUtils::createSamplerLayoutBinding(
      VkDescriptorSetLayoutBinding& layout
) {

   layout.binding = 1;
   layout.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   layout.descriptorCount = 1;
   layout.pImmutableSamplers = nullptr;
   layout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
}

void descriptorTypeUtils::createDescriptorImageInfo(
      const VkImageView& textureImageView,
      const VkSampler& textureSampler,
      VkDescriptorImageInfo& imageInfo
) {
   imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   imageInfo.imageView = textureImageView;
   imageInfo.sampler = textureSampler;
}

void descriptorTypeUtils::createSamplerWriteInfo(
      const VkDescriptorImageInfo& imageInfo,
      const VkDescriptorSet& descriptorSet,
      VkWriteDescriptorSet& descriptorWrite
) {
   descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrite.dstSet = descriptorSet;
   descriptorWrite.dstBinding = 1;
   descriptorWrite.dstArrayElement = 0;
   descriptorWrite.descriptorType = (
         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
   );
   descriptorWrite.descriptorCount = 1;
   descriptorWrite.pImageInfo = &imageInfo;
}
