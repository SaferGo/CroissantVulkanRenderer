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
set(TEXTURES_DIR "${CMAKE_SOURCE_DIR}/assets/textures")
set(MODEL_DIR "${CMAKE_SOURCE_DIR}/assets/obj")

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

# -STB(Image Loader) -> Header file that contains all the code.
set(STB_DIR "${PROJECT_LIBRARIES_DIR}/stb")
add_subdirectory(${STB_DIR} EXCLUDE_FROM_ALL)

# -ImGUI
# (compiles it as an static library)
set (IMGUI_DIR "${PROJECT_LIBRARIES_DIR}/imgui")
file(GLOB IMGUI_SOURCES
   ${IMGUI_DIR}/*.cpp
   ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp 
   ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp 
)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(
   imgui
   PUBLIC
      ${IMGUI_DIR}
      ${IMGUI_DIR}/backends
)

# -TinyObjLoader
set(TINY_OBJ_LOADER_DIR "${PROJECT_LIBRARIES_DIR}/tinyobjloader")
add_subdirectory(${TINY_OBJ_LOADER_DIR})

# - PThreads
set(THREADS_PREFER_PTRHEAD_FLAG ON)
find_package(Threads REQUIRED)

##################################Shaders######################################

# - Compilation of shaders
add_subdirectory(${SHADERS_SOURCE_DIR})
# To get the full path of the compiled shaders in the c++ code.
add_definitions(-DSHADERS_BINARY_DIR="${SHADERS_BINARY_DIR}")

################################Definitions####################################
add_definitions(-DTEXTURES_DIR="${TEXTURES_DIR}/")
add_definitions(-DMODEL_DIR="${MODEL_DIR}/")
#################################Executable####################################

add_executable(${PROJECT_NAME} 
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/main.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Renderer.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Window/Window.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/GUI/GUI.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/ValidationLayers/vlManager.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/QueueFamily/QueueFamilyIndices.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/QueueFamily/QueueFamilyHandles.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/QueueFamily/qfUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Swapchain/Swapchain.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/ShaderManager/shaderManager.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/GraphicsPipeline/GraphicsPipeline.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/RenderPass/RenderPass.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/RenderPass/attachmentUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/RenderPass/subPassUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Commands/CommandPool.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Commands/commandUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Device/Device.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Extensions/extensionsUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Buffers/bufferManager.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Buffers/bufferUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Descriptors/DescriptorSets.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Descriptors/DescriptorPool.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Descriptors/DescriptorTypes/UBO.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Descriptors/DescriptorTypes/Sampler.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Descriptors/DescriptorTypes/descriptorTypesUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Descriptors/descriptorSetLayoutUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Images/imageManager.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Textures/Texture.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/DepthBuffer/DepthBuffer.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/DepthBuffer/depthUtils.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Model/Vertex.cpp"
   "${PROJECT_SOURCE_DIR}/VulkanToyRenderer/Model/Model.cpp"
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
   stb
   tinyobjloader
   imgui
   PRIVATE ${CMAKE_DL_LIBS}
)
# CMAKE_DL_LIBS -> is the library libdl which helps to link dynamic
# libraries. We need it in order to use Vulkan Loader.
