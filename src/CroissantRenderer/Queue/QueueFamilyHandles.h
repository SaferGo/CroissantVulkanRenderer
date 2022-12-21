#pragma once

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Queue/QueueFamilyIndices.h>

struct QueueFamilyHandles
{
   VkQueue graphicsQueue;
   VkQueue presentQueue;
   VkQueue computeQueue;

   void setQueueHandles(
         const VkDevice& logicalDevice,
         const QueueFamilyIndices& qfIndices
   );
};
