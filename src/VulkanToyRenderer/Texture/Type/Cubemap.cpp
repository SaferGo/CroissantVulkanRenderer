#include <VulkanToyRenderer/Texture/Type/Cubemap.h>

#include <VulkanToyRenderer/Texture/mipmapUtils.h>
#include <VulkanToyRenderer/Texture/Bitmap.h>
#include <VulkanToyRenderer/Texture/cubemapUtils.h>
#include <VulkanToyRenderer/Image/imageManager.h>
#include <VulkanToyRenderer/Buffer/bufferManager.h>
#include <VulkanToyRenderer/Command/CommandPool.h>
#include <VulkanToyRenderer/Descriptor/Types/Sampler/Sampler.h>

Cubemap::Cubemap(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const TextureToLoadInfo& textureInfo,
      const std::string& textureFolderName,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      VkQueue& graphicsQueue,
      const UsageType& usage
) : Texture(
      logicalDevice,
      TextureType::CUBEMAP,
      samplesCount,
      textureInfo.desiredChannels,
      usage
    )
{
   m_mipLevels = 1;

   const std::string pathToTexture = (
         std::string(SKYBOX_DIR) + textureFolderName
   );

   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;

   const float* img = stbi_loadf(
         (pathToTexture + "/" + textureInfo.name).c_str(),
         &m_width,
         &m_height,
         &m_channels,
         // Desired channels
         // (we'll later convert it to 4)
         3
   );

   if (img == nullptr)
   {
      throw std::runtime_error(
            "Failed to load texture image: " +
            std::string(pathToTexture) + "/" +
            textureInfo.name
      );
   }

   // TODO: Verify if it's already created.
   //if (m_usage)
   //{
   //cubemapUtils::createIrradianceHDR(
   //      img,
   //      m_width,
   //      m_height,
   //      pathToTexture + "/Irradiance.hdr"
   //);
   //}

   // Converts RGB -> RGBA
   // (Because Vulkan doesn't accept to use RGB format as sampler)
   std::vector<float> img32(m_width * m_height * m_desiredChannels);
   cubemapUtils::float24to32(m_width, m_height, img, img32.data());
   stbi_image_free((void*)img);


   Bitmap in(m_width, m_height, 4, eBitmapFormat_Float, img32.data());
   Bitmap out = cubemapUtils::convertEquirectangularMapToVerticalCross(in);

   Bitmap cubemap = cubemapUtils::convertVerticalCrossToCubeMapFaces(out);

   uint8_t* data = cubemap.data_.data();
   uint32_t imageSize = (
         cubemap.w_ *
         cubemap.h_ *
         // rgba
         4 *
         Bitmap::getBytesPerComponent(cubemap.fmt_) *
         // faces
         6
   );

   // Creates the textures of the faces.

   bufferManager::createBuffer(
         physicalDevice,
         m_logicalDevice,
         imageSize,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         stagingBufferMemory,
         stagingBuffer
   );

   bufferManager::fillBuffer(
         m_logicalDevice,
         data,
         0,
         imageSize,
         stagingBufferMemory
   );
   
   m_image = Image(
         physicalDevice,
         m_logicalDevice,
         cubemap.w_,
         cubemap.h_,
         textureInfo.format,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         true,
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
   
   // We will transfer the pixels to the image object with a cmd buffer.
   // (staging buffer to the image obj)
   transitionImageLayout(
         textureInfo.format,
         // Since the image was created with the VK_IMAGE_LAYOUT_UNDEFINED
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         commandPool,
         graphicsQueue
   );
   imageManager::copyBufferToImage(
         cubemap.w_,
         cubemap.h_,
         true,
         graphicsQueue,
         stagingBuffer,
         commandPool,
         m_image.get()
   );

   // Another transition to sample from the shader.
   transitionImageLayout(
         textureInfo.format,
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
         commandPool,
         graphicsQueue
   );

   vkDestroyBuffer(
         m_logicalDevice,
         stagingBuffer,
         nullptr
   );
   vkFreeMemory(
         m_logicalDevice,
         stagingBufferMemory,
         nullptr
   );
   
}

Cubemap::~Cubemap() {}
