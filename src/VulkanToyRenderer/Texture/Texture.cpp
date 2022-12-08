#include <VulkanToyRenderer/Texture/Texture.h>

#include <iostream>

#include <vulkan/vulkan.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <VulkanToyRenderer/Texture/mipmapUtils.h>
#include <VulkanToyRenderer/Texture/Bitmap.h>
#include <VulkanToyRenderer/Command/commandManager.h>
#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Descriptor/Types/Sampler/Sampler.h>

Texture::Texture(
      const VkDevice& logicalDevice,
      const TextureType& type,
      const VkSampleCountFlagBits& samplesCount,
      const int& desiredChannels,
      const UsageType& usage
) : m_logicalDevice(logicalDevice),
    m_type(type),
    m_usage(usage),
    m_desiredChannels(desiredChannels),
    m_samplesCount(samplesCount)
{}

const VkImageView& Texture::getImageView() const
{
   return m_image.getImageView();
}

const VkSampler& Texture::getSampler() const
{
   return m_image.getSampler();
}

const UsageType& Texture::getUsage() const
{
   return m_usage;
}

void Texture::destroy()
{
   m_image.destroy();
}

Texture::~Texture() {}
