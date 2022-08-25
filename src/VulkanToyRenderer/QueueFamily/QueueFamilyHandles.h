#pragma once

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/QueueFamily/QueueFamilyIndices.h>

struct QueueFamilyHandles
{
   VkQueue graphicsQueue;
   VkQueue presentQueue;

   void setQueueHandles(
         const VkDevice& logicalDevice,
         const QueueFamilyIndices& qfIndices
   );
};
