#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace DescriptorTypes
{
   struct alignas(16) Material
   {
      glm::vec4 ambient;
      glm::vec4 diffuse;
      glm::vec4 specular;
      int shininess;
   };

   namespace UniformBufferObject
   {
      struct alignas(16) Normal
      {
         glm::mat4 model;
         glm::mat4 view;
         glm::mat4 proj;
         glm::vec4 lightPositions[10];
         glm::vec4 lightColors[10];
         glm::vec4 cameraPos;
         Material material;
         int lightsCount;
      };
      struct alignas(16) Light
      {
         glm::mat4 model;
         glm::mat4 view;
         glm::mat4 proj;
         glm::vec4 lightColor;
      };
   }
};
