#include <VulkanToyRenderer/Features/featuresUtils.h>

#include <vector>
#include <stdexcept>

VkSampleCountFlagBits featuresUtils::getMaxUsableSampleCount(
      const VkPhysicalDevice& physicalDevice
) {
   VkPhysicalDeviceProperties physicalDeviceProperties;
   vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

   VkSampleCountFlags counts = (
         physicalDeviceProperties.limits.framebufferColorSampleCounts &
         physicalDeviceProperties.limits.framebufferDepthSampleCounts
   );

   if (counts & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
   if (counts & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
   if (counts & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
   if (counts & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
   if (counts & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
   if (counts & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

   return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat featuresUtils::findSupportedFormat(
      const VkPhysicalDevice& physicalDevice,
      const std::vector<VkFormat>& candidates,
      const VkImageTiling tiling,
      const VkFormatFeatureFlags features
) {
   for (VkFormat format : candidates)
   {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR &&
         (props.linearTilingFeatures & features) == features
      ) {
         return format;
      } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (props.optimalTilingFeatures & features) == features
      )
         return format;
   }

   throw std::runtime_error("Failed to find supported format!");
}

 void featuresUtils::createDepthStencilStateInfo(
       const GraphicsPipelineType& type,
       VkPipelineDepthStencilStateCreateInfo& depthStencil
) {
    depthStencil.sType = (
         VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
   );
    // Specifies if the depth of new fragments shoud be compared to the depth
    // buffer to see if they should be discarded.
    depthStencil.depthTestEnable = VK_TRUE;
    // Specifies if the new depth of fragments that pass the depth test should
    // actually be written to the depth buffer.
    depthStencil.depthWriteEnable = VK_TRUE;

    // Specifies the comparasion that is performed to keep or discard
    // fragments. We're sticking to the convention of lower depth = closer,
    // so the depth of new fragments should be less.
    if (type == GraphicsPipelineType::SKYBOX ||
        type == GraphicsPipelineType::SHADOWMAP
    ) {
      depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    } else
      depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    // These 3 param. are used for the optional depth bound test(allows to
    // keep fragments that fall within the specified depth range).
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    // These 3 param. configure the stencil buffer operations.
    // (Not used for now...)
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};
}
