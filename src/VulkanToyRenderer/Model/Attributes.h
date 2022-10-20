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
         glm::vec2 texCoord;
         glm::vec3 normal;
         glm::vec3 tangent;
         glm::vec3 bitangent;
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

   namespace LIGHT
   {
      struct Vertex
      {
         glm::vec3 pos;
         glm::vec2 texCoord;
         glm::vec3 normal;
      };
      VkVertexInputBindingDescription getBindingDescription();
      std::vector<VkVertexInputAttributeDescription> 
            getAttributeDescriptions();
   };
};
