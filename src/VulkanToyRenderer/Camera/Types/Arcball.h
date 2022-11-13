#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Camera/Camera.h>

class Arcball : public Camera
{

public:

   Arcball(
         GLFWwindow* window,
         const glm::fvec4& pos,
         const float FOV,
         const float ratio,
         const float zNear,
         const float zFar
   );
   ~Arcball() override;
   void saveCursorPos();
   void updateCameraPos(glm::mat4& newRot);

private: 

   glm::fvec3 getArcballVector(const float x, const float y);

   glm::fvec2 m_lastCursorPos;
   glm::fvec2 m_currentCursorPos;
};
