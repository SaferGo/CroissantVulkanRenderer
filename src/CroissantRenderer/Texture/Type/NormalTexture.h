#pragma once

#include <CroissantRenderer/Texture/Texture.h>

#include <string>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Command/CommandPool.h>
#include <CroissantRenderer/Descriptor/Types/Sampler/Sampler.h>
#include <CroissantRenderer/Image/Image.h>

class NormalTexture : public Texture
{

public:

   NormalTexture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      const VkQueue& graphicsQueue,
      const UsageType& usage = UsageType::TO_COLOR
   );
   ~NormalTexture() override;

private:

};
