#include <VulkanToyRenderer/Camera/Camera.h>

#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>

Camera::Camera() {}

Camera::Camera(
      GLFWwindow* window,
      const CameraType& type,
      const float FOV,
      const float ratio,
      const float zNear,
      const float zFar
) : m_opWindow(window),
    m_type(type),
    m_FOV(FOV),
    m_ratio(ratio),
    m_zNear(zNear),
    m_zFar(zFar)
{
   m_proj = UBOutils::getUpdatedProjMatrix(
         glm::radians(m_FOV),
         m_ratio,
         m_zNear,
         m_zFar
   );
}

Camera::~Camera() {}

const CameraType Camera::getType() const
{
   return m_type;
}

const glm::mat4& Camera::getProjectionM() const
{
   return m_proj;
}

const float& Camera::getFOV() const
{
   return m_FOV;
}

void Camera::setFOV(const float newFOV)
{
   m_FOV = newFOV;

   m_proj = UBOutils::getUpdatedProjMatrix(
         glm::radians(m_FOV),
         m_ratio,
         m_zNear,
         m_zFar
   );
}
