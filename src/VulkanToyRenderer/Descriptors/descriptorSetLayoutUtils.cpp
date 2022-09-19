#include <VulkanToyRenderer/Descriptors/descriptorSetLayoutUtils.h>

#include <stdexcept>

/*
 * Combines all the different types of descriptors layouts in one
 * descriptor SET layout.
 */
void descriptorSetLayoutUtils::createDescriptorSetLayout(
      const VkDevice& logicalDevice,
      const std::vector<DescriptorInfo>& descriptorsInfo,
      VkDescriptorSetLayout& descriptorSetLayout
) {
   if (descriptorsInfo.size() == 0)
      throw std::runtime_error("Failed to create descriptor set layout!");

   // Descriptor bindings layouts
   std::vector<VkDescriptorSetLayoutBinding> bindings(descriptorsInfo.size());

   for (size_t i = 0; i < bindings.size(); i++)
   {
      createDescriptorBindingLayout(
            descriptorsInfo[i],
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
      const DescriptorInfo& descriptorInfo,
      const std::vector<VkSampler>& immutableSamplers,
      VkDescriptorSetLayoutBinding& layout
) {
   // Binding used in the shader.
   layout.binding = descriptorInfo.binding;
   layout.descriptorType = descriptorInfo.type;
   layout.descriptorCount = 1;
   layout.stageFlags = descriptorInfo.shaderStage;
   layout.pImmutableSamplers = immutableSamplers.data();
}
