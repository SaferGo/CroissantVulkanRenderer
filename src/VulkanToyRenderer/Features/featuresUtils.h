#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/GraphicsPipeline/GraphicsPipeline.h>

namespace featuresUtils
{
   void createDepthStencilStateInfo(
       const GraphicsPipelineType& type,
       VkPipelineDepthStencilStateCreateInfo& depthStencil
   );
   
   VkFormat findSupportedFormat(
         const VkPhysicalDevice& phyisicalDevice,
         const std::vector<VkFormat>& candidates,
         const VkImageTiling tiling,
         const VkFormatFeatureFlags features
   );
   
   
   VkSampleCountFlagBits getMaxUsableSampleCount(
      const VkPhysicalDevice& physicalDevice
   );
};
