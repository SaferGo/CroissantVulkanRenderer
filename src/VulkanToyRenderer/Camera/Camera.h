#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

enum class CameraType
{
   ARCBALL = 0
};

class Camera
{

public:

   Camera();
   Camera(
         GLFWwindow* window,
         const CameraType& type,
         const float FOV,
         const float ratio,
         const float zNear,
         const float zFar
   );
   virtual ~Camera() = 0;
   const CameraType getType() const;
   const glm::mat4& getProjectionM() const;
   const float& getFOV() const;
   void setFOV(const float newFOV);

protected:

   // Observer pointer
   GLFWwindow* m_opWindow;
   CameraType m_type;
   
   float m_FOV;
   float m_ratio;
   float m_zNear;
   float m_zFar;

   glm::mat4 m_proj;

private:

};
