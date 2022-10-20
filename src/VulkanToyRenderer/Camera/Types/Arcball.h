#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <VulkanToyRenderer/Camera/Camera.h>

class Arcball : public Camera
{

public:

   Arcball(
         GLFWwindow* window,
         const float FOV,
         const float ratio,
         const float zNear,
         const float zFar
   );
   ~Arcball() override;

   void saveCursorPos();

   void updateCameraPos(const glm::mat4& view, glm::mat4& newRot);

private: 

   glm::fvec3 getArcballVector(const float x, const float y);

   glm::fvec2 m_lastCursorPos;
   glm::fvec2 m_currentCursorPos;
};
