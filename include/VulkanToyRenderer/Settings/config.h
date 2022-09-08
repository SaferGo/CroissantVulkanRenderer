#pragma once

#include <vulkan/vulkan.h>

namespace config
{
   // Window
   inline const uint16_t RESOLUTION_W = 800;
   inline const uint16_t RESOLUTION_H = 600;

   inline const char* TITLE = "Hello Vulkan";

   // Graphic's settings
   inline const int MAX_FRAMES_IN_FLIGHT = 2;
   inline const VkClearValue CLEAR_COLOR = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
};
