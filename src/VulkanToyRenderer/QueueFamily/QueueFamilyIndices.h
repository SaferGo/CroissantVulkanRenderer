#pragma once

#include <optional>
#include <vector>

#include <vulkan/vulkan.h>

/*
 * - graphicsFamily -> Queue that suports graphics commands.
 * - presentFamily  -> Queue that supports sending/presenting frames into the
 *                     window.
 */

// List of indices of the Queue famlies that we required.
struct QueueFamilyIndices
{
   std::optional<uint32_t> graphicsFamily;
   std::optional<uint32_t> presentFamily;
   bool areAllQueueFamiliesSupported;

   void getIndicesOfRequiredQueueFamilies (
         const VkPhysicalDevice& physicalDevice,
         const VkSurfaceKHR& surface
   );
};
