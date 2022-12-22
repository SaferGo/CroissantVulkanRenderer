#pragma once

#include <CroissantRenderer/Model/Model.h>

struct UBOinfo
{
   const glm::vec4& cameraPos;
   const glm::mat4& view;
   const glm::mat4& proj;
   const glm::mat4& lightSpace;
   const uint32_t& lightsCount;
   const VkExtent2D& extent;
};
