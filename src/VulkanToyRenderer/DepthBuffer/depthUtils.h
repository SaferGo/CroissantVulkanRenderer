#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace depthUtils
{
   void createDepthStencilStateInfo(
         VkPipelineDepthStencilStateCreateInfo& depthStencil
   );

   VkFormat findSupportedFormat(
         const VkPhysicalDevice& phyisicalDevice,
         const std::vector<VkFormat>& candidates,
         const VkImageTiling tiling,
         const VkFormatFeatureFlags features
   );
};
