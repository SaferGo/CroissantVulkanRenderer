#include <VulkanToyRenderer/Descriptors/descriptorSetLayoutUtils.h>

#include <stdexcept>

/*
 * Combines all the different types of descriptors layouts in one
 * descriptor SET layout.
 */
void descriptorSetLayoutUtils::createDescriptorSetLayout(
      const VkDevice& logicalDevice,
      const std::vector<VkDescriptorType>& descriptorTypes,
      const std::vector<uint32_t>& descriptorBindings,
      const std::vector<VkShaderStageFlagBits>& descriptorStages,
      VkDescriptorSetLayout& descriptorSetLayout
) {
   if (descriptorTypes.size() == 0 ||
       descriptorBindings.size() == 0 ||
       descriptorStages.size() == 0 ||
       descriptorTypes.size() != descriptorBindings.size() ||
       descriptorTypes.size() != descriptorStages.size()
   ) {
      throw std::runtime_error("Failed to create descriptor set layout!");
   }

   // Descriptor bindings layouts
   std::vector<VkDescriptorSetLayoutBinding> bindings(descriptorTypes.size());

   for (size_t i = 0; i < bindings.size(); i++)
   {
      createDescriptorBindingLayout(
            descriptorBindings[i],
            descriptorTypes[i],
            descriptorStages[i],
            {},
            bindings[i]
      );
   }

   VkDescriptorSetLayoutCreateInfo layoutInfo{};
   layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
   layoutInfo.pBindings = bindings.data();

   auto status = vkCreateDescriptorSetLayout(
         logicalDevice,
         &layoutInfo,
         nullptr,
         &descriptorSetLayout
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create descriptor set layout!");
}

void descriptorSetLayoutUtils::destroyDescriptorSetLayout(
      const VkDevice& logicalDevice,
      VkDescriptorSetLayout& descriptorSetLayout
) {
   vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
}

void descriptorSetLayoutUtils::createDescriptorBindingLayout(
      const uint32_t bindingNumber,
      const VkDescriptorType& type,
      const VkShaderStageFlags& stageFlags,
      const std::vector<VkSampler>& immutableSamplers,
      VkDescriptorSetLayoutBinding& layout
) {
   // Binding used in the shader.
   layout.binding = bindingNumber;
   layout.descriptorType = type;
   layout.descriptorCount = 1;
   layout.stageFlags = stageFlags;
   layout.pImmutableSamplers = immutableSamplers.data();
}
