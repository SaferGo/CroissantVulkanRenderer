#pragma once

#include <VulkanToyRenderer/Texture/Texture.h>

#include <string>

#include <vulkan/vulkan.h>

#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Descriptor/Types/Sampler/Sampler.h>
#include <VulkanToyRenderer/Image/Image.h>

class Cubemap : public Texture
{

public:

   Cubemap(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const std::string& textureFolderName,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      const VkQueue& graphicsQueue,
      const UsageType& usage = UsageType::TO_COLOR
   );
   ~Cubemap() override;

private:

};
