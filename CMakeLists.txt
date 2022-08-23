
cmake_minimum_required(VERSION 3.9.1)

project(ToyRenderer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin")
set(PROJECT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
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

#################################Executable####################################

add_executable(${PROJECT_NAME} 
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/main.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/HelloTriangleApp.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/ValidationLayersManager/vlManager.cpp"
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
