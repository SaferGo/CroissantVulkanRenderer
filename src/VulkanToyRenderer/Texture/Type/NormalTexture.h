#pragma once

#include <VulkanToyRenderer/Texture/Texture.h>

#include <string>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Descriptor/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Image/Image.h>

class NormalTexture : public Texture
{

public:

   NormalTexture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue,
      const UsageType& usage = UsageType::TO_COLOR
   );
   ~NormalTexture() override;

private:

};
