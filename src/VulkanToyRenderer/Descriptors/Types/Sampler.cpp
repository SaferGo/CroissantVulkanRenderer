#include <VulkanToyRenderer/Descriptors/Types/Sampler.h>

#include <stdexcept>

#include <vulkan/vulkan.h>

Sampler::Sampler() {}
Sampler::~Sampler() {}

void Sampler::createSampler(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice
) {
   VkSamplerCreateInfo samplerInfo{};
   samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   // Specifies how to interpolate texels that are magnified or minified.
   // Manification -> when oversampling.
   // Minification -> when undersampling.
   samplerInfo.magFilter = VK_FILTER_LINEAR;
   samplerInfo.minFilter = VK_FILTER_LINEAR;
   samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
   samplerInfo.anisotropyEnable = VK_TRUE;

   // Limits the amount of texel samples that can be used to calculate the
   // final color.
   // (A lower value results in better performance, but lower quality results)
   // (To find the best match, we will retrieve the Phyisical Device properties)
   VkPhysicalDeviceProperties properties{};
   vkGetPhysicalDeviceProperties(physicalDevice, &properties);
   samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

   // Specifies the color that is returned when sampling beyond the image with
   // clmap to border adressing mode.
   samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
   // Vk_TRUE: we can use the coords within [0, texWidth) and [0, texHeight)
   // Vk_FALSE: we can use the coords within [0, 1)
   samplerInfo.unnormalizedCoordinates = VK_FALSE;
   // These two options are used in SHADOW MAPS(percentage-closer filtering).
   samplerInfo.compareEnable = VK_FALSE;
   samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
   // Mipmapping fields:
   samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
   samplerInfo.mipLodBias = 0.0f;
   samplerInfo.minLod = 0.0f;
   samplerInfo.maxLod = 0.0f;

   auto status = vkCreateSampler(
         logicalDevice,
         &samplerInfo,
         nullptr,
         &m_sampler
   );

   if (status != VK_SUCCESS)
      throw std::runtime_error("Failed to create texture sampler!");
}


const VkSampler& Sampler::getSampler() const
{
   return m_sampler;
}

void Sampler::destroySampler(const VkDevice& logicalDevice)
{
   vkDestroySampler(logicalDevice, m_sampler, nullptr);
}
