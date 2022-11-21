#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace DescriptorTypes
{
   namespace UniformBufferObject
   {
      struct alignas(16) LightInfo
      {
         glm::vec4 pos;
         glm::vec4 dir;
         glm::vec4 color;
         float attenuation;
         float radius;
         float intensity;
         int type;
      };

      struct alignas(16) NormalPBR
      {
         glm::mat4 model;
         glm::mat4 view;
         glm::mat4 proj;
         glm::mat4 lightSpace;
         glm::vec4 cameraPos;
         int lightsCount;
         bool hasNormalMap;
      };

      struct alignas(16) Light
      {
         glm::mat4 model;
         glm::mat4 view;
         glm::mat4 proj;
         glm::vec4 lightColor;
      };

      struct alignas(16) Skybox
      {
         glm::mat4 model;
         glm::mat4 view;
         glm::mat4 proj;
      };

      struct alignas(16) ShadowMap
      {
         glm::mat4 model;
         glm::mat4 lightSpace;
      };
   }
};
