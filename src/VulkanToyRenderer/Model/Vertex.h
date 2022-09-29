#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct Vertex
{
   glm::vec3 pos;
   glm::vec3 color;
   glm::vec2 texCoord;
   glm::vec3 normal;

   
   bool operator==(const Vertex& other) const;
   static VkVertexInputBindingDescription getBindingDescription();
   static std::vector<VkVertexInputAttributeDescription>
      getAttributeDescriptions();
};
