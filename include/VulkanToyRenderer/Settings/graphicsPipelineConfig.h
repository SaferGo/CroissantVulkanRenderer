#pragma once

namespace GRAPHICS_PIPELINE
{
   /////////////////////////////For PBR Models/////////////////////////////////

   namespace PBR
   {

      inline const std::vector<DescriptorInfo> UBOS_INFO = {
         {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_VERTEX_BIT |
                  VK_SHADER_STAGE_FRAGMENT_BIT
            )
         }
      };
      inline const std::vector<DescriptorInfo> SAMPLERS_INFO = {
         {
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_FRAGMENT_BIT
            )
         },
         {
            2,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_FRAGMENT_BIT
            )
         },
         {
            3,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_FRAGMENT_BIT
            )

         }
      };

      inline const int TEXTURES_PER_MESH_COUNT = SAMPLERS_INFO.size();
      inline const int UBOS_PER_MESH_COUNT = UBOS_INFO.size();

   };

   ///////////////////////////////For Skyboxes/////////////////////////////////
   namespace SKYBOX
   {
      
      inline const std::vector<DescriptorInfo> UBOS_INFO = {
         {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_VERTEX_BIT
            )
         }
      };

      inline const std::vector<DescriptorInfo> SAMPLERS_INFO = {
         {
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_FRAGMENT_BIT
            )
         }
      };

      inline const int TEXTURES_PER_MESH_COUNT = SAMPLERS_INFO.size();
      inline const int UBOS_PER_MESH_COUNT = UBOS_INFO.size();
   };

   /////////////////////////////For Light Models///////////////////////////////

   namespace LIGHT
   {
      inline const std::vector<DescriptorInfo> UBOS_INFO = {
         {
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_VERTEX_BIT |
                  VK_SHADER_STAGE_FRAGMENT_BIT
            )
         }
      };
      inline const std::vector<DescriptorInfo> SAMPLERS_INFO = {
         {
            1,
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            (VkShaderStageFlagBits)(
                  VK_SHADER_STAGE_FRAGMENT_BIT
            )
         }
      };

      inline const int TEXTURES_PER_MESH_COUNT = SAMPLERS_INFO.size();
      inline const int UBOS_PER_MESH_COUNT = UBOS_INFO.size();
   };
};
