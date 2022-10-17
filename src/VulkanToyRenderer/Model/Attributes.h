#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace Attributes
{
   namespace PBR
   {
      struct Vertex
      {
         glm::vec3 pos;
         glm::vec3 color;
         glm::vec2 texCoord;
         glm::vec3 normal;
      };

      VkVertexInputBindingDescription getBindingDescription();
      std::vector<VkVertexInputAttributeDescription> 
         getAttributeDescriptions();
   };

   namespace SKYBOX
   {
      struct Vertex
      {
         glm::vec3 pos;
      };

      VkVertexInputBindingDescription getBindingDescription();
      std::vector<VkVertexInputAttributeDescription> 
            getAttributeDescriptions();

   };
};
