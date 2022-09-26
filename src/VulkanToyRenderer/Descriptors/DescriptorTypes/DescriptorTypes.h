#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace DescriptorTypes
{
   struct UniformBufferObject
   {
      alignas(16) glm::mat4 model;
      alignas(16) glm::mat4 view;
      alignas(16) glm::mat4 proj;
      alignas(16) glm::vec3 lightPositions;
      alignas(16) glm::vec3 lightColors;
      alignas(16) int       lightsCount;
   };

};
