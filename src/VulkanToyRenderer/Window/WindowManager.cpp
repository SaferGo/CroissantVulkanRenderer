#include <VulkanToyRenderer/Window/WindowManager.h>

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

WindowManager::WindowManager() {}

void WindowManager::createWindow(
      const uint16_t width,
      const uint16_t height,
      const char* title
) {
   glfwInit();
   // Since GLFW automatically creates an OpenGL context,
   // we need to not create it.
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   // ----------------------------------------------------------------
   // Paramet. #1 -> Width.
   // Paramet. #2 -> Height.
   // Paramet. #3 -> Title.
   // Paramet. #4 -> Specifies in which monitor to open the window.
   // Paramet. #5 -> Irrelevant to Vulkan.
   // ----------------------------------------------------------------
   m_window = glfwCreateWindow(
         width,
         height,
         title,
         nullptr,
         nullptr
   );
}

const VkSurfaceKHR WindowManager::getSurface() const
{
   return m_surface;
}

GLFWwindow* WindowManager::getWindow()
{
   return m_window;
}

void WindowManager::getResolutionInPixels(int& width, int& height) const
{
   glfwGetFramebufferSize(m_window, &width, &height);
}

void WindowManager::createSurface(const VkInstance& instance)
{
   if (glfwCreateWindowSurface(instance, m_window, nullptr, &m_surface))
      throw std::runtime_error("Failed to create a window surface");
}

bool WindowManager::isWindowClosed() const
{
   return glfwWindowShouldClose(m_window);
}

/*
 * The range of possible resolutions by the Windows is in "capabilities", but
 * if the capabilitie's width or height is equal to the max value of uint32_t
 * that means that the resolution can be modified.
 */
bool WindowManager::isAllowedToModifyTheResolution(
      const VkSurfaceCapabilitiesKHR& capabilities
) const {
   return (
         capabilities.currentExtent.width ==
         std::numeric_limits<uint32_t>::max()
   );
}

void WindowManager::pollEvents()
{
   glfwPollEvents();
}

void WindowManager::destroySurface(const VkInstance& instance)
{
   vkDestroySurfaceKHR(instance, m_surface, nullptr);
}

void WindowManager::destroyWindow()
{
   glfwDestroyWindow(m_window);
   glfwTerminate();
}

WindowManager::~WindowManager() {}