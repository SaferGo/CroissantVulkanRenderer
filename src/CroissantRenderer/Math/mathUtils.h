#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mathUtils
{
   glm::mat4 getUpdatedModelMatrix(
         const glm::fvec4 actualPos,
         const glm::fvec3 actualRot,
         const glm::fvec3 actualSize
   );
   glm::mat4 getUpdatedProjMatrix(
      const float vfov,
      const float aspect,
      const float nearZ,
      const float farZ
   );
};
