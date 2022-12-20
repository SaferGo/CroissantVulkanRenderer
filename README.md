<p align="center">
  <img src="https://user-images.githubusercontent.com/19228971/208704463-5fc58dc3-59a2-4814-b315-0f5835a4a155.png">
</p>
<h1 align="center"> Croissant - Real-Time Vulkan Renderer</h1>
<p align="center"> Real-time, Physically based renderer built with Vulkan and modern C++20.</p>


## About
The aim of this project was to build a proper render graph to be able to play and learn vulkan and at the same time learn graphics techniques.

## Warning
This project is under development, expect API changes and missing features.

## Render Samples
![1gg](https://user-images.githubusercontent.com/19228971/208722571-807d33d1-5da5-4118-9529-bf0ed000291f.png)
![3c](https://user-images.githubusercontent.com/19228971/208724316-87dfa63a-009f-4d35-baeb-b79d53ebc0b4.png)

## Repository structure

```bash
Croissant
|-- assets              
|   |-- models              # Meshes and textures
|   `-- skybox              # HDR files with their generated irradiance and BRDFlut textures
|
|-- bin                     # Contains the executable files
|
|-- build                   # CMake compile
|
|-- include                 # Project header files
|   `-- Settings            # All user tweakable settings files(scene, camera, pipeline, etc)
|
|-- libs                    # Dependencies
|   |-- ASSIMP
|   |-- GLFW
|   |-- GLI
|   |-- GLM
|   |-- Dear imgui           
|   |-- stb_image
|   |-- tracy
|   |-- Vulkan-Loader            
|   `-- Vulkan-Tools
|
|-- shaders
|
|-- src                     # C++ implementation files
|   |-- Buffer
|   |-- Camera
|   |-- Command
|   |-- Computation
|   |-- Descriptor
|   |-- Device
|   |-- Features
|   |-- Framebuffer
|   |-- GUI
|   |-- Image
|   |-- Math
|   |-- Model
|   |-- Pipeline
|   |-- Queue
|   |-- Renderer
|   |-- RenderPass
|   |-- Scene
|   |-- Shader
|   |-- Swapchain
|   |-- Texture
|   |-- VkInstance
|   `-- Window
|   
`-- CMakeLists.txt          # CMake build script
```

## Third party libraries
Here's the list of the libraries included in the project:

* [ASSIMP](https://github.com/assimp/assimp): Mesh and material loading.
* [GLFW](https://github.com/glfw/glfw): A multi-platform library for window and input.
* [GLI](https://github.com/g-truc/gli): Image library(used to generate the BRDFlut texture).
* [GLM](https://github.com/g-truc/glm): Mathematics library for graphics software.
* [Dear ImGui,](https://github.com/ocornut/imgui): GUI.
* [stb_image](https://github.com/nothings/stb): Image loading/decoding.
* [Tracy](https://github.com/wolfpld/tracy): Frame profiler.
* [Vulkan-Loader](https://github.com/KhronosGroup/Vulkan-Loader)
* [Vulkan-Tools](https://github.com/KhronosGroup/Vulkan-Tools): Validation Layers.

# Tested toolchains

| Compiler            | Operating System                     | Architecture |
|---------------------|--------------------------------------|--------------|
| GCC 9.4.0           | Linux Ubuntu 20.04.4 with kernel 5.8.0-53 | x64 |
| ?           | Windows 11 (will be added soon) | x64 |

## Included assets
The following assets are bundled with the project:

- "Cerberus" gun model by [Andrew Maximov](http://artisaverb.info).
- HDR environment map by [Bob Groothuis](http://www.bobgroothuis.com/blog/) obtained from [HDRLabs sIBL archive](http://www.hdrlabs.com/sibl/archive.html) (distributed under [CC-BY-NC-SA 3.0](https://creativecommons.org/licenses/by-nc-sa/3.0/us/)).
