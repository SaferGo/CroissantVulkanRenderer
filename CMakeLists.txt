cmake_minimum_required(VERSION 3.9.1)

project(Croissant)

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
set(MODEL_DIR "${CMAKE_SOURCE_DIR}/assets/models")
set(SKYBOX_DIR "${CMAKE_SOURCE_DIR}/assets/skybox")

if (CMAKE_BUILD_TYPE MATCHES Debug)
   set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
else ()
   set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")
   add_definitions(-DRELEASE_MODE_ON=0)
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

# - STB(Image Loader) -> Header file that contains all the code.
set(STB_DIR "${PROJECT_LIBRARIES_DIR}/stb")
add_subdirectory(${STB_DIR} EXCLUDE_FROM_ALL)

# - ImGUI
# (It compiles it as an static library)
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

# - Assimp
set(ASSIMP_DIR "${PROJECT_LIBRARIES_DIR}/assimp")
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(${ASSIMP_DIR})

# - PThreads
set(THREADS_PREFER_PTRHEAD_FLAG ON)
find_package(Threads REQUIRED)

# - Gli
set(GLI_DIR "${PROJECT_LIBRARIES_DIR}/gli")
add_subdirectory(${GLI_DIR} EXCLUDE_FROM_ALL)

# - Tracy
option(TRACY_ENABLE "" ON)
option(TRACY_ON_DEMAND "" ON)
add_subdirectory("${PROJECT_LIBRARIES_DIR}/tracy")

##################################Shaders######################################

# - Compilation of shaders
add_subdirectory(${SHADERS_SOURCE_DIR})
# To get the full path of the compiled shaders in the c++ code.
add_definitions(-DSHADERS_BINARY_DIR="${SHADERS_BINARY_DIR}")

################################Definitions####################################
add_definitions(-DTEXTURES_DIR="${TEXTURES_DIR}/")
add_definitions(-DMODEL_DIR="${MODEL_DIR}/")
add_definitions(-DSKYBOX_DIR="${SKYBOX_DIR}/")
#################################Executable####################################

add_executable(${PROJECT_NAME} 
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/main.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Renderer/Renderer.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Window/Window.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/GUI/GUI.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Computation/Computation.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/VKinstance/VKinstance.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/VKinstance/ValidationLayers/vlManager.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/VKinstance/extensionsUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Queue/QueueFamilyIndices.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Queue/QueueFamilyHandles.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Queue/queueFamilyUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Swapchain/Swapchain.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Shader/shaderManager.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Pipeline/Pipeline.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Pipeline/Graphics.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Pipeline/Compute.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/RenderPass/RenderPass.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/RenderPass/attachmentUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/RenderPass/subPassUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Command/CommandPool.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Command/commandManager.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Device/Device.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Buffer/bufferManager.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Buffer/bufferUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Descriptor/DescriptorSets.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Descriptor/DescriptorPool.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Descriptor/Types/UBO/UBO.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Descriptor/Types/UBO/UBOutils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Descriptor/Types/Sampler/Sampler.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Descriptor/Types/descriptorTypesUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Descriptor/descriptorSetLayoutManager.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Image/imageManager.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Image/Image.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Texture/Texture.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Texture/Type/Cubemap.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Texture/Type/NormalTexture.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Texture/mipmapUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Texture/cubemapUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Texture/Bitmap.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Model/Attributes.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Model/Model.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Model/Types/NormalPBR.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Model/Types/Skybox.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Model/Types/Light.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Camera/Camera.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Camera/Types/Arcball.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Features/ShadowMap.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Features/MSAA.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Features/DepthBuffer.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Features/featuresUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Features/PrefilteredEnvMap.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Framebuffer/framebufferManager.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Math/mathUtils.cpp"
   "${PROJECT_SOURCE_DIR}/CroissantRenderer/Scene/Scene.cpp"
)

target_include_directories(
   ${PROJECT_NAME}
   PUBLIC
      "${PROJECT_INCLUDE_DIR}"
      "${VULKAN_INCLUDE_DIR}"
      "${GLFW_INCLUDE_DIRS}"
      "${TracyClient_INCLUDE_DIRS}"
   PRIVATE
      "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(${PROJECT_NAME}
   PUBLIC
   glfw 
   ${Vulkan_LIBRARY}
   Threads::Threads  # Needed by GLFW.
   X11               # To create windows. Needed by GLFW.
   glm
   stb
   assimp
   imgui
   Tracy::TracyClient
   gli
   PRIVATE
   ${CMAKE_DL_LIBS}
)
# CMAKE_DL_LIBS -> is the library libdl which helps to link dynamic
# libraries. We need it in order to use Vulkan Loader.
