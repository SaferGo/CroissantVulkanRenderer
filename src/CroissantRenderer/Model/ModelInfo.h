#pragma once

#include <CroissantRenderer/Model/Model.h>

enum class LightType
{
   DIRECTIONAL_LIGHT = 0,
   POINT_LIGHT       = 1,
   SPOT_LIGHT        = 2,
   NONE              = 3
};

struct ModelInfo
{
   ModelType   type;
   std::string name;
   std::string folderName;
   std::string fileName;
   glm::fvec3  color;
   glm::fvec3  pos;
   glm::fvec3  rot;
   glm::fvec3  size;

   // For light models.
   LightType   lType;
   glm::fvec3  endPos;
};
