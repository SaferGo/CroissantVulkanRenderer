#pragma once

#include <VulkanToyRenderer/Model/Model.h>

struct UBOinfo
{
   const glm::vec4& cameraPos;
   const glm::mat4& view;
   const glm::mat4& proj;
   const glm::mat4& lightSpace;
   const int& lightsCount;
   const VkExtent2D& extent;
};
