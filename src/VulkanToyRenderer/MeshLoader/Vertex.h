#pragma once

#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct Vertex
{
   glm::vec2 pos;
   glm::vec3 color;

   static VkVertexInputBindingDescription getBindingDescription();
   static std::array<VkVertexInputAttributeDescription, 2>
      getAttributeDescriptions();
};
