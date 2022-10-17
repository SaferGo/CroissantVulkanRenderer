#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace DescriptorTypes
{
   namespace UniformBufferObject
   {
      struct alignas(16) NormalPBR
      {
         glm::mat4 model;
         glm::mat4 view;
         glm::mat4 proj;
         glm::vec4 lightPositions[10];
         glm::vec4 lightColors[10];
         glm::vec4 cameraPos;
         int lightsCount;
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
   }
};
