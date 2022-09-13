#pragma once

#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>


struct Vertex
{
   glm::vec3 pos;
   glm::vec3 color;
   glm::vec2 texCoord;

   bool operator==(const Vertex& other) const;
   static VkVertexInputBindingDescription getBindingDescription();
   static std::array<VkVertexInputAttributeDescription, 3>
      getAttributeDescriptions();
};

