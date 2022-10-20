#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>

namespace UBOutils
{   glm::mat4 getUpdatedModelMatrix(
         const glm::fvec4 actualPos,
         const glm::fvec3 actualRot,
         const glm::fvec3 actualSize
   );
   glm::mat4 getUpdatedViewMatrix(
         const glm::fvec3& cameraPos,
         const glm::fvec3& centerPos,
         const glm::fvec3& upAxis
   );
   glm::mat4 getUpdatedProjMatrix(
      const float vfov,
      const float aspect,
      const float nearZ,
      const float farZ
   );

   template<typename T>
   void updateUBO(
         UBO& ubo,
         const VkDevice& logicalDevice,
         T& newUbo,
         const uint32_t& currentFrame
   );
};
