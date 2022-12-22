<p align="center">
  <img src="https://user-images.githubusercontent.com/19228971/208704463-5fc58dc3-59a2-4814-b315-0f5835a4a155.png">
</p>
<h1 align="center"> Croissant - Real-Time Vulkan Renderer</h1>
<p align="center">
  <img src="https://img.shields.io/badge/language-C%2B%2B20-brightgreen">
  <img src="https://img.shields.io/badge/platforms-Linux-brightgreen">
  <img src="https://img.shields.io/github/license/inexorgame/vulkan-renderer?color=brightgreen">
</p>
<p align="center"> Real-time, Physically based renderer built with Vulkan and modern C++17.</p>


## About
The aim of this project was to create a properly functioning render graph that would allow me to learn and experiment with Vulkan, as well as provide an opportunity to learn graphics techniques.

## Disclaimer
This project is under development, expect API changes, bugs and missing features.

## Render Samples
![1gg](https://user-images.githubusercontent.com/19228971/208722571-807d33d1-5da5-4118-9529-bf0ed000291f.png)
![3c](https://user-images.githubusercontent.com/19228971/208724316-87dfa63a-009f-4d35-baeb-b79d53ebc0b4.png)

## Features
**Renderer**
<ul>
  <li> Physically based rendering(Cook–Torrance BRDF) </li>
  <li> Image based lighting </li>
  <li> Forward rendering </li>
  <li> Multisample anti-aliasing (MSAA) </li>
  <li> HDRI skymap loading </li>
  <li> Compute shaders </li>
  <li> Shadow mapping </li>
  <li> Normal mapping </li>
  <li> Different light types (directional, point and spot lights) </li>
  <li> Texture mipmaps </li>
  <li> Arcball camera </li>
  <li> GUI </li>
</ul>

**Other Features**
<ul>
  <li> Multi-threaded texture asset importing </li> 
  <li> Model loading </li>
</ul>

## Repository structure

```c++
Croissant
|
|-- assets              
|   |-- models              # Meshes and textures
|   `-- skybox              # HDR files with their generated irradiance, prefiltered env. and BRDFlut textures
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
* [ImGui](https://github.com/ocornut/imgui): GUI.
* [stb_image](https://github.com/nothings/stb): Image loading/decoding.
* [Tracy](https://github.com/wolfpld/tracy): Frame profiler.
* [Vulkan-Loader](https://github.com/KhronosGroup/Vulkan-Loader)
* [Vulkan-Tools](https://github.com/KhronosGroup/Vulkan-Tools): Validation Layers.

## Render Graph
![Render Graph (5)](https://user-images.githubusercontent.com/19228971/209051311-cfe64c87-710d-4b91-aeb9-86c881e39406.png)
![Render Graph (6)](https://user-images.githubusercontent.com/19228971/209051351-86eb79c7-b32f-44f4-b185-132790dc7297.png)

## Usage

#### Warning
- The renderer currently only works with a dedicated graphics card, but I will add compatibility with integrated GPUs in the future.
- To start the renderer, you need to add one skybox, one directional light, and at least one model.
- To add a model or skybox to the render, add the folder in 'assets'.

```cpp
/* Commands:
*   
*   - addSkybox(fileName, folderName);
*   - addObjectPBR(name, folderName, fileName, position, rotation, size);
*   - addDirectionalLight(name, folderName, fileName, color, position, targetPosition, size);
*   - addSpotLight(name, folderName, fileName, color, position, targetPosition, rotation, size);
*   - addPointLight(name, folderName, fileName, color, position, size);
*
*   - demo1(); // Damaged Helmet
*   - demo2(); // AK 47
*   - demo3(); // Collier Flintlock Revolver
*   - demo4(); // Sponza day
*   - demo5(); // Sponza night
*   - demo6(); // Metal Rough Spheres
*/

int main()
{
   Renderer app;

   try
   {
      // Scene
      {
         app.addSkybox("fileName.hdr", "folderName");
         app.addObjectPBR(
               "name",
               "folderName",
               "fileName",
               glm::fvec3(0.0f), // Position
               glm::fvec3(0.0f), // Rotation
               glm::fvec3(1.0f)  // Size
         );
         app.addDirectionalLight(
               "name",
               "folderName",
               "fileName",
               glm::fvec3(1.0f), // Color
               glm::fvec3(0.0f), // Position
               glm::fvec3(1.0f), // Target Position
               glm::fvec3(1.0f)  // Size
         );
      }

      app.run();

   } catch (const std::exception& e)
   {
      std::cerr << e.what() << "\n";

      return 0;
   }

   return 0;
}
```

## Controls

Input        | Action
-------------|-------
RMB drag     | Rotate camera
Scroll wheel | Zoom in/out

## Dependencies
- cmake >= 3.9.1
- gcc >= 9.4.0
- vulkan-validationlayers-dev
- spirv-tools
- glslc

## Building on Linux
```diff
$ git clone --recurse-submodules https://github.com/SaferGo/CroissantRenderer.git
$ cd CroissantRenderer/build
$ bash buildReleaseMode.sh
// or buildDebugMode.sh
```
After a successful build, the resulting executable can be found in the bin directory.

## Tested toolchains

| Compiler            | Operating System                     | Architecture |
|---------------------|--------------------------------------|--------------|
| GCC 9.4.0           | Linux Ubuntu 20.04.4 with kernel 5.8.0-53 | x64 |
| ?           | Windows 11 (will be added soon) | x64 |

## References
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [3D Graphics Rendering Cookbook: A comprehensive guide to exploring rendering algorithms in modern OpenGL and Vulkan](https://amzn.eu/d/hpsSViV)
- [Vulkan Cookbook: Solutions to next gen 3D graphics API](https://amzn.eu/d/hZXOS0R)
- [Hybrid Rendering Engine by Angelo1211](https://github.com/Angelo1211/HybridRenderingEngine)
- [Vulkan physically-Based Rendering by SaschaWillems](https://github.com/SaschaWillems/Vulkan-glTF-PBR)
- [Shadow Mapping in Vulkan by igalia](https://blogs.igalia.com/itoral/2017/07/30/working-with-lights-and-shadows-part-ii-the-shadow-map/)
- [Integrating Dear ImGui in a custom Vulkan renderer](https://frguthmann.github.io/posts/vulkan_imgui/)

## Included assets
The following assets are bundled with the project:

### - HDRs
- Winter Forest and Apartment from [ihdri](https://www.ihdri.com/).
- Country Club, Farm Field, Neon Photostudio, Peppermint Powerplant 2 and Shangai Bund from [PolyHaven](https://polyhaven.com/) (distributed under [CC0](https://polyhaven.com/license)).
- Arches PineTree from [HdrLabs](http://www.hdrlabs.com/) (distributed under [Creative Commons Attribution-Noncommercial-Share Alike 3.0 License](https://creativecommons.org/licenses/by-nc-sa/3.0/us/)).
### - Models
- Cube from [Cesium](https://cesium.com/) (distributed under [Creative Commons Attribution 4.0 International License](https://creativecommons.org/licenses/by/4.0/)).
- Damaged Helmet from [theblueturtle_](https://sketchfab.com/theblueturtle_) (distributed under [Creative Commons Attribution-Noncommercial-Share Alike 3.0 License](https://creativecommons.org/licenses/by-nc-sa/3.0/us/)).
- Sponza from [alexandre-pestana](https://www.alexandre-pestana.com/).
- Metal Rough Spheres from Analytical Graphics (distributed under [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/)).
- AK 47 Tactical Upgrade from [Mateusz Woliński](https://sketchfab.com/jeandiz) (distributed under [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/)).
- Collier Flintlock Revolver from [Artem Goyko](https://sketchfab.com/Artem.Goyko) (distributed under [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/)).
