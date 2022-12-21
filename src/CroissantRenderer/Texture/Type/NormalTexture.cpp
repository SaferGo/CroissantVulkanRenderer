#include <CroissantRenderer/Texture/Type/NormalTexture.h>

#include <iostream>

#include <vulkan/vulkan.h>

#include <CroissantRenderer/Texture/mipmapUtils.h>
#include <CroissantRenderer/Texture/Bitmap.h>
#include <CroissantRenderer/Image/imageManager.h>
#include <CroissantRenderer/Buffer/bufferManager.h>
#include <CroissantRenderer/Command/commandManager.h>
#include <CroissantRenderer/Command/CommandPool.h>
#include <CroissantRenderer/Descriptor/Types/Sampler/Sampler.h>

/*
 * Creates all the texture resources.
 */
NormalTexture::NormalTexture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      const VkQueue& graphicsQueue,
      const UsageType& usage
) : Texture(
      logicalDevice,
      TextureType::NORMAL_TEXTURE,
      samplesCount,
      textureInfo.desiredChannels,
      usage
    )
{

   std::string pathToTexture;
   VkDeviceSize imageSize;
   uint8_t* pixels;

   if (usage == UsageType::TO_COLOR)
   {
      pathToTexture = (
            std::string(MODEL_DIR) +
            textureInfo.folderName + "/" +
            textureInfo.name
      );

      pixels = stbi_load(
            pathToTexture.c_str(),
            &m_width,
            &m_height,
            &m_channels,
            STBI_rgb_alpha
      );

      if (!pixels)
      {
         throw std::runtime_error(
               "Failed to load texture image: " + std::string(pathToTexture)
         );
      }

      m_mipLevels = mipmapUtils::getAmountOfSupportedMipLevels(
            m_width,
            m_height
      );

   } else if (usage == UsageType::BRDF)
   {

      pathToTexture = (
            std::string(SKYBOX_DIR) +
            textureInfo.folderName + "/" +
            textureInfo.name
      );

      gli::texture pixelsTmp = gli::load_ktx(pathToTexture.c_str());
      glm::tvec3<uint32_t> extent(pixelsTmp.extent(0));

      pixels = (uint8_t*)pixelsTmp.data(0, 0, 0);

      m_width = extent.x;
      m_height = extent.y;
      m_channels = 2;

      m_mipLevels = 1;

   } else {
      throw std::runtime_error("Unknown UsageType for texture creation");
   }

   imageSize = m_width * m_height * m_desiredChannels;

   m_image = Image(
         physicalDevice,
         m_logicalDevice,
         m_width,
         m_height,
         textureInfo.format,
         VK_IMAGE_TILING_OPTIMAL,
         (
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT
         ),
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         false,
         m_mipLevels,
         m_samplesCount,
         VK_IMAGE_ASPECT_COLOR_BIT,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_SAMPLER_ADDRESS_MODE_REPEAT,
         VK_FILTER_LINEAR
   );
 
   imageManager::copyDataToImage(
         // Staging Buffer Info
         physicalDevice,
         m_logicalDevice,
         imageSize,
         0,
         pixels,
         // Image info
         m_width,
         m_height,
         textureInfo.format,
         m_mipLevels,
         false,
         graphicsQueue,
         commandPool,
         m_image.get()
   );     

   mipmapUtils::generateMipmaps(
         physicalDevice,
         commandPool,
         graphicsQueue,
         m_image.get(),
         m_width,
         m_height,
         textureInfo.format,
         m_mipLevels
   );
}

NormalTexture::~NormalTexture() {}
