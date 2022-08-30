cmake_minimum_required(VERSION 3.9.1)

project(ToyRenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(PROJECT_OUTPUT_DIR "${CMAKE_BINARY_DIR}/../bin")
set(PROJECT_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR})
set(PROJECT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(SHADERS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADERS_BINARY_DIR "${PROJECT_BIN_DIR}/shaders")
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(PROJECT_LIBRARIES_DIR "${CMAKE_SOURCE_DIR}/libs")

if (CMAKE_BUILD_TYPE MATCHES Debug)
   set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
   #else ()
   #   set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")
endif ()

#####################################Libs######################################
#- Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)

# - Vulkan Tools
set(VULKAN_TOOLS_DIR "${PROJECT_LIBRARIES_DIR}/Vulkan-Tools")
add_subdirectory(${VULKAN_TOOLS_DIR} EXCLUDE_FROM_ALL)

# - Vulkan Loader (like GLEW for OpenGL)
set(VULKAN_LOADER_DIR "${PROJECT_LIBRARIES_DIR}/Vulkan-Loader")
add_subdirectory(${VULKAN_LOADER_DIR} EXCLUDE_FROM_ALL)

# - GLFW
set(GLFW_DIR "${PROJECT_LIBRARIES_DIR}/glfw")
add_subdirectory(${GLFW_DIR} EXCLUDE_FROM_ALL)

# - GLM
set(GLM_DIR "${PROJECT_LIBRARIES_DIR}/glm")
add_subdirectory(${GLM_DIR} EXCLUDE_FROM_ALL)

# - PThreads
set(THREADS_PREFER_PTRHEAD_FLAG ON)
find_package(Threads REQUIRED)

##################################Shaders######################################

# - Compilation of shaders
add_subdirectory(${SHADERS_SOURCE_DIR})
# To get the full path of the compiled shaders in the c++ code.
add_definitions(-DSHADERS_BINARY_DIR="${SHADERS_BINARY_DIR}")

#################################Executable####################################

add_executable(${PROJECT_NAME} 
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/main.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Renderer.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Window/WindowManager.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/ValidationLayers/vlManager.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/QueueFamily/QueueFamilyIndices.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/QueueFamily/QueueFamilyHandles.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/QueueFamily/qfUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Swapchain/SwapchainManager.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/ShaderManager/shaderManager.cpp"
)

target_include_directories(
   ${PROJECT_NAME}
   PUBLIC
      "${PROJECT_INCLUDE_DIR}"
      "${VULKAN_INCLUDE_DIR}"
      "${GLFW_INCLUDE_DIRS}"
   PRIVATE
      "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME} PUBLIC
   glfw 
   ${Vulkan_LIBRARY}
   Threads::Threads  # Needed by GLFW.
   X11               # To create windows. Needed by GLFW.
   glm
   PRIVATE ${CMAKE_DL_LIBS}
)
# CMAKE_DL_LIBS -> is the library libdl which helps to link dynamic
# libraries. We need it in order to use Vulkan Loader.
