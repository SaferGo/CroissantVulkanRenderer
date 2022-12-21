#include <CroissantRenderer/Math/mathUtils.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
 * Remember that the correct order is SRT!
 */
glm::mat4 mathUtils::getUpdatedModelMatrix(
      const glm::fvec4 actualPos,
      const glm::fvec3 actualRot,
      const glm::fvec3 actualSize
) {
   glm::mat4 model = glm::mat4(1.0);
   
   model = glm::translate(
         model,
         glm::vec3(actualPos)
   );

   model = glm::rotate(
         model,
         actualRot.x,
         glm::vec3(1.0f, 0.0f, 0.0f)
   );

   model = glm::rotate(
         model,
         actualRot.y,
         glm::vec3(0.0f, 1.0f, 0.0f)
   );

   model = glm::rotate(
         model,
         actualRot.z,
         glm::vec3(0.0f, 0.0f, 1.0f)
   );

   model = glm::scale(
         model,
         actualSize
   );
   
   return model;
}

glm::mat4 mathUtils::getUpdatedProjMatrix(
      const float vfov,
      const float aspect,
      const float nearZ,
      const float farZ
) {
   glm::mat4 proj = glm::perspective(
         vfov,
         aspect,
         nearZ,
         farZ
   );

   // GLM was designed for OpenGl, where the Y coordinate of the clip coord. is
   // inverted. To compensate for that, we have to flip the sign on the scaling
   // factor of the Y axis.
   proj[1][1] *= -1;

   return proj;
}
