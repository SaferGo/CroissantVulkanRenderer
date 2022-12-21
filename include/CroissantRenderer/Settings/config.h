#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Descriptor/DescriptorInfo.h>

namespace config
{
   // Window
   inline const uint16_t RESOLUTION_W = 1366;
   inline const uint16_t RESOLUTION_H = 768;

   inline const char* WINDOW_TITLE = "Croissant";

   // Graphic's settings
   inline const int MAX_FRAMES_IN_FLIGHT = 2;
   inline const float FOV = 45.0f;
   inline const float Z_NEAR = 0.01f;
   inline const float Z_FAR = 200.0f;

   // Scene
   inline const uint32_t LIGHTS_COUNT = 10;

   // BRDF
   inline const uint32_t BRDF_WIDTH  = 256;
   inline const uint32_t BRDF_HEIGHT = 256;

   // Prefiltered Env. Map
   inline const uint32_t PREF_ENV_MAP_DIM = 512;
};
