#include <VulkanToyRenderer/Descriptors/DescriptorSets.h>

#include <vector>
#include <array>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Descriptors/DescriptorInfo.h>
#include <VulkanToyRenderer/Descriptors/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptors/Types/descriptorTypesUtils.h>
#include <VulkanToyRenderer/Settings/config.h>

/*
 * Creates, allocates and configures the descriptor sets.
 */
void DescriptorSets::createDescriptorSets(
      const VkDevice logicalDevice,
      const std::vector<DescriptorInfo>& uboInfo,
      const std::vector<DescriptorInfo>& samplersInfo,
      const std::vector<Texture>& textures,
      std::vector<VkBuffer>& uniformBuffers,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool
) {
   // TODO: Improve this, since all the descriptors sets
   // are the same, just copy it instead of creating them
   // many times.

   std::vector<VkDescriptorImageInfo> imageInfos;
   imageInfos.resize(textures.size());

   // It creates a descriptor set for each frame in flight.
   // ONE FRAME FOR WRITING, ANOTHER FOR READING
   m_descriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
   m_descriptorSetLayouts.resize(
         config::MAX_FRAMES_IN_FLIGHT,
         descriptorSetLayout
   );

   descriptorPool.allocDescriptorSets(
         logicalDevice,
         m_descriptorSets,
         m_descriptorSetLayouts
   );

   // Configures the descriptor sets
   for (size_t i = 0; i < m_descriptorSets.size(); i++)
   {

      VkDescriptorBufferInfo bufferInfo{};
      descriptorTypesUtils::createDescriptorBufferInfo(
            uniformBuffers[i],
            bufferInfo
      );

      for (size_t j = 0; j < imageInfos.size(); j++)
      {
         descriptorTypesUtils::createDescriptorImageInfo(
               textures[j].getTextureImageView(),
               textures[j].getTextureSampler(),
               imageInfos[j]
         );
      }

      // Describes how to update the descriptors.
      // (how and which buffer/image use to bind with the each descriptor)
      std::vector<VkWriteDescriptorSet> descriptorWrites;
      descriptorWrites.resize(uboInfo.size() + samplersInfo.size());

      // UBOs
      for (size_t j = 0; j < uboInfo.size(); j++)
      {
         createDescriptorWriteInfo(
               bufferInfo,
               m_descriptorSets[i],
               uboInfo[j].bindingNumber,
               0,
               uboInfo[j].descriptorType,
               descriptorWrites[j]
         );
      }
      // Samplers
      for (size_t j = 0; j < samplersInfo.size(); j++)
      {
         // Another
         createDescriptorWriteInfo(
               imageInfos[j],
               m_descriptorSets[i],
               // Binding number.
               samplersInfo[j].bindingNumber,
               0,
               samplersInfo[j].descriptorType,
               descriptorWrites[uboInfo.size() + j]
         );
      }

      vkUpdateDescriptorSets(
         logicalDevice,
         static_cast<uint32_t>(descriptorWrites.size()),
         descriptorWrites.data(),
         0,
         nullptr
      );
   }
}

template<typename T>
void DescriptorSets::createDescriptorWriteInfo(
      const T& descriptorInfo,
      const VkDescriptorSet& descriptorSet,
      const uint32_t& dstBinding,
      const uint32_t& dstArrayElement,
      const VkDescriptorType& type,
      VkWriteDescriptorSet& descriptorWrite
) {
   descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrite.dstSet = descriptorSet;
   descriptorWrite.dstBinding = dstBinding;
   descriptorWrite.dstArrayElement = dstArrayElement;
   descriptorWrite.descriptorType = type;
   descriptorWrite.descriptorCount = 1;

   if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      descriptorWrite.pBufferInfo = (VkDescriptorBufferInfo*)&descriptorInfo;
   if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      descriptorWrite.pImageInfo = (VkDescriptorImageInfo*)&descriptorInfo;
}

//////////////////////////////////////Instances////////////////////////////////

template void DescriptorSets::createDescriptorWriteInfo<VkDescriptorBufferInfo>(
      const VkDescriptorBufferInfo& descriptorInfo,
      const VkDescriptorSet& descriptorSet,
      const uint32_t& dstBinding,
      const uint32_t& dstArrayElement,
      const VkDescriptorType& type,
      VkWriteDescriptorSet& descriptorWrite
);

template void DescriptorSets::createDescriptorWriteInfo<VkDescriptorImageInfo>(
      const VkDescriptorImageInfo& descriptorInfo,
      const VkDescriptorSet& descriptorSet,
      const uint32_t& dstBinding,
      const uint32_t& dstArrayElement,
      const VkDescriptorType& type,
      VkWriteDescriptorSet& descriptorWrite
);


///////////////////////////////////////////////////////////////////////////////

const VkDescriptorSet& DescriptorSets::get(
      const uint32_t index
) const {
   return m_descriptorSets[index];
}
