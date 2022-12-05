#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Descriptor/DescriptorInfo.h>
#include <VulkanToyRenderer/Descriptor/DescriptorPool.h>
#include <VulkanToyRenderer/Descriptor/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Texture/Texture.h>

struct DescriptorSetInfo
{
   const Texture*               envMap;
   const Texture*               irradianceMap;
   const Texture*               BRDFlut;
   const VkImageView*           shadowMapView;
   const VkSampler*             shadowMapSampler;
};

class DescriptorSets
{

public:

   DescriptorSets();
   DescriptorSets(
         const VkDevice logicalDevice,
         const std::vector<DescriptorInfo>& bindingUBOs,
         const std::vector<DescriptorInfo>& bindingSamplers,
         const std::vector<std::shared_ptr<Texture>>& textures,
         std::vector<UBO*>& UBOs,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorPool& descriptorPool,
         DescriptorSetInfo* additionalTextures = nullptr
   );
   DescriptorSets(
      const VkDevice logicalDevice,
      const std::vector<DescriptorInfo>& buffersInfo,
      const std::vector<VkBuffer>& buffers,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool
   );
   DescriptorSets(const DescriptorSets& other);
   DescriptorSets& operator=(const DescriptorSets& other);
   ~DescriptorSets();

   const VkDescriptorSet& get(const uint32_t index) const;

private:

   template<typename T>
   void createDescriptorWriteInfo(
         const T& descriptorInfo,
         const VkDescriptorSet& descriptorSet,
         const uint32_t& dstBinding,
         const uint32_t& dstArrayElement,
         const VkDescriptorType& type,
         VkWriteDescriptorSet& descriptorWrite
   );


   std::vector<VkDescriptorSet>       m_descriptorSets;
   std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;

};
