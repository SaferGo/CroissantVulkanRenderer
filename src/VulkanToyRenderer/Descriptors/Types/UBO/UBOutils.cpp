#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VulkanToyRenderer/Descriptors/Types/UBO/UBO.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Model/Model.h>

/*
 * Remember that the correct order is SRT!
 */
glm::mat4 UBOutils::getUpdatedModelMatrix(
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

glm::mat4 UBOutils::getUpdatedViewMatrix(
      const glm::fvec3& cameraPos,
      const glm::fvec3& centerPos,
      const glm::fvec3& upAxis
) {
   return glm::lookAt(
         cameraPos,
         centerPos,
         upAxis
   );
}

glm::mat4 UBOutils::getUpdatedProjMatrix(
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

template<typename T>
void UBOutils::updateUBO(
      UBO& ubo,
      const VkDevice& logicalDevice,
      T& newUbo,
      const uint32_t& currentFrame
) {
   void* data;
   vkMapMemory(
         logicalDevice,
         ubo.getUniformBufferMemory(currentFrame),
         0,
         sizeof(newUbo),
         0,
         &data
   );
      memcpy(data, &newUbo, sizeof(newUbo));
   vkUnmapMemory(
         logicalDevice,
         ubo.getUniformBufferMemory(currentFrame)
   );
}
//////////////////////////////////Instances////////////////////////////////////
template void UBOutils::updateUBO<
   DescriptorTypes::UniformBufferObject::NormalPBR
>(
      UBO& ubo,
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject::NormalPBR& newUbo,
      const uint32_t& currentFrame
);
template void UBOutils::updateUBO<
   DescriptorTypes::UniformBufferObject::Light
>(
      UBO& ubo,
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject::Light& newUbo,
      const uint32_t& currentFrame
);
template void UBOutils::updateUBO<
   DescriptorTypes::UniformBufferObject::Skybox
>(
      UBO& ubo,
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject::Skybox& newUbo,
      const uint32_t& currentFrame
);
///////////////////////////////////////////////////////////////////////////////


