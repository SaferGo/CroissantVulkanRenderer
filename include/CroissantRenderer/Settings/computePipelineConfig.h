#pragma once

namespace COMPUTE_PIPELINE
{
   namespace BRDF
   {
      inline const std::vector<DescriptorInfo> BUFFERS_INFO = {
         {
            0,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_COMPUTE_BIT
            )
         },
         {
            1,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_COMPUTE_BIT
            )
         }

      };
   };
};
