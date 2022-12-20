#include <VulkanToyRenderer/Descriptor/DescriptorSets.h>

#include <vector>
#include <array>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Descriptor/DescriptorInfo.h>
#include <VulkanToyRenderer/Descriptor/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptor/Types/descriptorTypesUtils.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>
#include <VulkanToyRenderer/Settings/config.h>


DescriptorSets::DescriptorSets() {}

/*
 * Creates, allocates and configures the descriptor sets.
 */
DescriptorSets::DescriptorSets(
      const VkDevice logicalDevice,
      const std::vector<DescriptorInfo>& uboInfo,
      const std::vector<DescriptorInfo>& samplersInfo,
      const std::vector<std::shared_ptr<Texture>>& textures,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool,
      DescriptorSetInfo* additionalTextures,
      const std::vector<UBO*>& UBOs
) {


   std::vector<VkDescriptorImageInfo> imageInfos;
   imageInfos.resize(samplersInfo.size());

   // It creates a descriptor set for each frame in flight.
   // ONE FRAME FOR WRITING, ANOTHER FOR READING
   m_descriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
   // We can't optimize this since Vulkan hasn't an optimization/function to
   // create descriptors sets with one same Descriptor Set Layout.
   m_descriptorSetLayouts.resize(
         config::MAX_FRAMES_IN_FLIGHT,
         descriptorSetLayout
   );

   descriptorPool.allocDescriptorSets(
         m_descriptorSetLayouts,
         m_descriptorSets
   );

   // Configures the descriptor sets
   for (size_t i = 0; i < m_descriptorSets.size(); i++)
   {

      std::vector<VkDescriptorBufferInfo> bufferInfos(UBOs.size());
      for (size_t j = 0; j < UBOs.size(); j++)
      {
         descriptorTypesUtils::createDescriptorBufferInfo(
               UBOs[j]->get(i),
               bufferInfos[j]
         );
      }

      // TODO: Improve this.

      // Samplers of textures
      for (size_t j = 0; j < textures.size(); j++)
      {
         descriptorTypesUtils::createDescriptorImageInfo(
               textures[j]->getImageView(),
               textures[j]->getSampler(),
               imageInfos[j]
         );
      }

      if (additionalTextures != nullptr)
      {
         descriptorTypesUtils::createDescriptorImageInfo(
               additionalTextures->envMap->getImageView(),
               additionalTextures->envMap->getSampler(),
               imageInfos[samplersInfo.size() - 5]
         );
         descriptorTypesUtils::createDescriptorImageInfo(
               additionalTextures->irradianceMap->getImageView(),
               additionalTextures->irradianceMap->getSampler(),
               imageInfos[samplersInfo.size() - 4]
         );
         descriptorTypesUtils::createDescriptorImageInfo(
               additionalTextures->BRDFlut->getImageView(),
               additionalTextures->BRDFlut->getSampler(),
               imageInfos[samplersInfo.size() - 3]
         );
         descriptorTypesUtils::createDescriptorImageInfo(
               additionalTextures->prefilteredEnvMap->getImageView(),
               additionalTextures->prefilteredEnvMap->getSampler(),
               imageInfos[samplersInfo.size() - 2]
         );


         descriptorTypesUtils::createDescriptorImageInfo(
               *(additionalTextures->shadowMapView),
               *(additionalTextures->shadowMapSampler),
               imageInfos[samplersInfo.size() - 1]
         );
      }

      // Describes how to update the descriptors.
      // (how and which buffer/image use to bind with the each descriptor)
      std::vector<VkWriteDescriptorSet> descriptorWrites(
            uboInfo.size() + samplersInfo.size()
      );

      // UBOs
      for (size_t j = 0; j < uboInfo.size(); j++)
      {
         createDescriptorWriteInfo(
               bufferInfos[j],
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

/*
 * Used for Compute Pipelines.
 */
DescriptorSets::DescriptorSets(
   const VkDevice logicalDevice,
   const std::vector<DescriptorInfo>& bufferInfos,
   const std::vector<VkBuffer>& buffers,
   const VkDescriptorSetLayout& descriptorSetLayout,
   DescriptorPool& descriptorPool
) {

   // We just need 1 descriptor set per compute pipeline.
   m_descriptorSets.resize(1);

   m_descriptorSetLayouts.resize(
         1,
         descriptorSetLayout
   );

   descriptorPool.allocDescriptorSets(
         m_descriptorSetLayouts,
         m_descriptorSets
   );

   std::vector<VkDescriptorBufferInfo> descriptorBufferInfos(buffers.size());
   for (size_t i = 0; i < buffers.size(); i++)
   {
      descriptorTypesUtils::createDescriptorBufferInfo(
            buffers[i],
            descriptorBufferInfos[i]
      );
   }

   std::vector<VkWriteDescriptorSet> descriptorWrites(buffers.size());
   for (size_t j = 0; j < buffers.size(); j++)
   {
      createDescriptorWriteInfo(
            descriptorBufferInfos[j],
            m_descriptorSets[0],
            bufferInfos[j].bindingNumber,
            0,
            bufferInfos[j].descriptorType,
            descriptorWrites[j]
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

DescriptorSets::DescriptorSets(const DescriptorSets& other)
   : m_descriptorSets(other.m_descriptorSets),
     m_descriptorSetLayouts(other.m_descriptorSetLayouts)
{}

DescriptorSets& DescriptorSets::operator=(const DescriptorSets& other)
{
   if (this == &other)
      return *this;

   m_descriptorSets = other.m_descriptorSets;
   m_descriptorSetLayouts = other.m_descriptorSetLayouts;

   return *this;
}

DescriptorSets::~DescriptorSets() {}

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

   if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
       type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
   ) {

      descriptorWrite.pBufferInfo = (VkDescriptorBufferInfo*)&descriptorInfo;

   } else if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
   {

      descriptorWrite.pImageInfo = (VkDescriptorImageInfo*)&descriptorInfo;
      
   } else {

      std::runtime_error("Error creating a descriptor set!");

   }
}

const VkDescriptorSet& DescriptorSets::get(
      const uint32_t index
) const {
   return m_descriptorSets[index];
}
