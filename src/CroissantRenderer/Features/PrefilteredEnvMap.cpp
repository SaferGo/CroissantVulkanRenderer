#include <CroissantRenderer/Features/PrefilteredEnvMap.h>

#include <iostream>

#include <CroissantRenderer/Settings/graphicsPipelineConfig.h>
#include <CroissantRenderer/Model/Attributes.h>
#include <CroissantRenderer/Model/ModelInfo.h>
#include <CroissantRenderer/Framebuffer/framebufferManager.h>
#include <CroissantRenderer/Image/imageManager.h>
#include <CroissantRenderer/RenderPass/attachmentUtils.h>
#include <CroissantRenderer/RenderPass/subPassUtils.h>
#include <CroissantRenderer/Texture/mipmapUtils.h>
#include <CroissantRenderer/Descriptor/DescriptorPool.h>
#include <CroissantRenderer/Command/commandManager.h>

template<typename T>
PrefilteredEnvMap<T>::PrefilteredEnvMap(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool,
      const uint32_t dim,
      const std::vector<Mesh<T>>& meshes,
      const std::shared_ptr<Texture>& envMap
)  : m_logicalDevice(logicalDevice), 
     m_dim(dim),
     m_format(VK_FORMAT_R16G16B16A16_SFLOAT)
{

   m_mipLevels = mipmapUtils::getAmountOfSupportedMipLevels(dim, dim);

   createTargetImage(physicalDevice);
   createRenderPass();
   createOffscreenFramebuffer(physicalDevice, graphicsQueue, commandPool);
   createPipeline();
   createDescriptorPool();
   createDescriptorSet(envMap);
   recordCommandBuffer(commandPool, graphicsQueue, meshes);

}

template<typename T>
void PrefilteredEnvMap<T>::createDescriptorPool()
{

   m_descriptorPool = DescriptorPool(
         m_logicalDevice,
         {
            {
               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
               2
            }
         },
         2
   );
}

template<typename T>
void PrefilteredEnvMap<T>::createDescriptorSet(
      const std::shared_ptr<Texture>& envMap
) {

   m_descriptorSets = DescriptorSets(
         m_logicalDevice,
         {},
         GRAPHICS_PIPELINE::PREFILTER_ENV_MAP::SAMPLERS_INFO,
         {envMap},
         m_graphicsPipeline.getDescriptorSetLayout(),
         m_descriptorPool
   );
}

template<typename T>
void PrefilteredEnvMap<T>::recordCommandBuffer(
   const std::shared_ptr<CommandPool>& commandPool,
   const VkQueue& graphicsQueue,
   const std::vector<Mesh<T>>& meshes
) {
   VkClearValue clearValues;
	clearValues.color = { { 0.0f, 0.0f, 0.2f, 0.0f } };
   std::vector<glm::mat4> matrices = {
      glm::rotate(
            glm::rotate(
               glm::mat4(1.0f),
               glm::radians(90.0f),
               glm::vec3(0.0f, 1.0f, 0.0f)
            ),
            glm::radians(180.0f),
            glm::vec3(1.0f, 0.0f, 0.0f)
      ),
   	glm::rotate(
            glm::rotate(
               glm::mat4(1.0f),
               glm::radians(-90.0f),
               glm::vec3(0.0f, 1.0f, 0.0f)
            ),
            glm::radians(180.0f),
            glm::vec3(1.0f, 0.0f, 0.0f)
      ),
   	glm::rotate(
            glm::mat4(1.0f),
            glm::radians(-90.0f),
            glm::vec3(1.0f, 0.0f, 0.0f)
      ),
   	glm::rotate(
            glm::mat4(1.0f),
            glm::radians(90.0f),
            glm::vec3(1.0f, 0.0f, 0.0f)
      ),
   	glm::rotate(
            glm::mat4(1.0f),
            glm::radians(180.0f),
            glm::vec3(1.0f, 0.0f, 0.0f)
      ),
   	glm::rotate(
            glm::mat4(1.0f),
            glm::radians(180.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
      ),
   };

   const VkCommandBuffer& commandBuffer = commandPool->getCommandBuffer(0);

   // Change image layout for all cubemap faces to transfer destination
   {
      commandPool->resetCommandBuffer(0);
      commandPool->beginCommandBuffer(0, commandBuffer);

         VkImageMemoryBarrier imgMemoryBarrier{};
         VkPipelineStageFlags sourceStage, destinationStage;
         imageManager::createImageMemoryBarrier(
               m_mipLevels,
               VK_IMAGE_LAYOUT_UNDEFINED,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               true,
               m_targetImage.get(),
               imgMemoryBarrier,
               sourceStage,
               destinationStage
         );

         commandManager::synchronization::recordPipelineBarrier(
               sourceStage,
               destinationStage,
               0,
               commandBuffer,
               {},
               {},
               {imgMemoryBarrier}
         );

      commandPool->endCommandBuffer(commandBuffer);
      commandPool->submitCommandBuffer(
            graphicsQueue,
            {commandBuffer},
            true,
            {},
            std::nullopt,
            {},
            std::nullopt
      );
   }

   for (uint32_t m = 0; m < m_mipLevels; m++)
   {
      for (uint32_t face = 0; face < 6; face++)
      {
         float viewportDim = static_cast<float>(
               m_dim * std::pow(0.5f, m)
         );

         commandPool->resetCommandBuffer(0);
         commandPool->beginCommandBuffer(0, commandBuffer);

            //---------------------------------CMDs----------------------------

            // Set Dynamic States
            commandManager::state::setViewport(
                  0.0f,
                  0.0f,
                  {
                     viewportDim,
                     viewportDim
                  },
                  0.0f,
                  1.0f,
                  0,
                  1,
                  commandBuffer
            );
            commandManager::state::setScissor(
                  {0, 0},
                  {
                     m_dim,
                     m_dim
                  },
                  0,
                  1,
                  commandBuffer
            );

            //--------------------------RenderPass--------------------------
            
			   // Render scene from cube face's point of view
            m_renderPass.begin(
                  m_framebuffer,
                  {
                     m_dim,
                     m_dim
                  },
                  {clearValues},
                  commandBuffer,
                  VK_SUBPASS_CONTENTS_INLINE
            );


               m_pushBlock.mvp = glm::perspective(
                     (float)(glm::pi<float>() / 2.0),
                     1.0f,
                     0.1f,
                     float(m_dim)
               ) * matrices[face];
               m_pushBlock.roughness = (float)m / float(m_mipLevels - 1);

					vkCmdPushConstants(
                     commandBuffer,
                     m_graphicsPipeline.getPipelineLayout(),
                     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                     0,
                     sizeof(PushBlockPrefilterEnv),
                     &m_pushBlock
               );

               commandManager::state::bindPipeline(
                     m_graphicsPipeline.get(),
                     PipelineType::GRAPHICS,
                     commandBuffer
               );

               commandManager::state::bindDescriptorSets(
                     m_graphicsPipeline.getPipelineLayout(),
                     PipelineType::GRAPHICS,
                     // Index of first descriptor set.
                     0,
                     {m_descriptorSets.get(0)},
                     // Dynamic offsets.
                     {},
                     commandBuffer
               );

               for (auto& mesh : meshes)
               {
                  commandManager::state::bindVertexBuffers(
                        {mesh.vertexBuffer},
                        // Offsets.
                        {0},
                        // Index of first binding.
                        0,
                        // Bindings count.
                        1,
                        commandBuffer
                  );
                  commandManager::state::bindIndexBuffer(
                        mesh.indexBuffer,
                        // Offset.
                        0,
                        VK_INDEX_TYPE_UINT32,
                        commandBuffer
                  );

                  commandManager::action::drawIndexed(
                        // Index Count
                        mesh.indices.size(),
                        // Instance Count
                        1,
                        // First index.
                        0,
                        // Vertex Offset.
                        0,
                        // First Intance.
                        0,
                        commandBuffer
                  );
               }

            m_renderPass.end(commandBuffer);

            {
               VkImageMemoryBarrier imgMemoryBarrier{};
               VkPipelineStageFlags sourceStage, destinationStage;
               imageManager::createImageMemoryBarrier(
                     1,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     false,
                     m_offscreenImage.get(),
                     imgMemoryBarrier,
                     sourceStage,
                     destinationStage
               );

               commandManager::synchronization::recordPipelineBarrier(
                     sourceStage,
                     destinationStage,
                     0,
                     commandBuffer,
                     {},
                     {},
                     {imgMemoryBarrier}
               );
            }

            copyRegionOfImage(face, m, viewportDim, commandBuffer);

            {
               VkImageMemoryBarrier imgMemoryBarrier{};
               VkPipelineStageFlags sourceStage, destinationStage;
               imageManager::createImageMemoryBarrier(
                     1,
                     VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                     false,
                     m_offscreenImage.get(),
                     imgMemoryBarrier,
                     sourceStage,
                     destinationStage
               );

               commandManager::synchronization::recordPipelineBarrier(
                     sourceStage,
                     destinationStage,
                     0,
                     commandBuffer,
                     {},
                     {},
                     {imgMemoryBarrier}
               );
            }

         commandPool->endCommandBuffer(commandBuffer);
         commandPool->submitCommandBuffer(
               graphicsQueue,
               {commandBuffer},
               true,
               {},
               std::nullopt,
               {},
               std::nullopt
         );
      }

   }

      {
         commandPool->resetCommandBuffer(0);
         commandPool->beginCommandBuffer(0, commandBuffer);

         VkImageMemoryBarrier imgMemoryBarrier{};
         VkPipelineStageFlags sourceStage, destinationStage;
         imageManager::createImageMemoryBarrier(
               m_mipLevels,
               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
               true,
               m_targetImage.get(),
               imgMemoryBarrier,
               sourceStage,
               destinationStage
         );

         commandManager::synchronization::recordPipelineBarrier(
               sourceStage,
               destinationStage,
               0,
               commandBuffer,
               {},
               {},
               {imgMemoryBarrier}
         );

         commandPool->endCommandBuffer(commandBuffer);
         commandPool->submitCommandBuffer(
               graphicsQueue,
               {commandBuffer},
               true,
               {},
               std::nullopt,
               {},
               std::nullopt
         );

      }
}

template<typename T>
void PrefilteredEnvMap<T>::copyRegionOfImage(
      float face,
      float mipLevel,
      float viewportDim,
      const VkCommandBuffer& commandBuffer
) {
   VkImageCopy copyRegion{};
   
	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.srcOffset = { 0, 0, 0 };

	copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.dstSubresource.baseArrayLayer = face;
	copyRegion.dstSubresource.mipLevel = mipLevel;
	copyRegion.dstSubresource.layerCount = 1;
	copyRegion.dstOffset = { 0, 0, 0 };

	copyRegion.extent.width = static_cast<uint32_t>(viewportDim);
	copyRegion.extent.height = static_cast<uint32_t>(viewportDim);
	copyRegion.extent.depth = 1;

   vkCmdCopyImage(
         commandBuffer,
         m_offscreenImage.get(),
         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
         m_targetImage.get(),
         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
         1,
         &copyRegion
   );
}

template<typename T>
void PrefilteredEnvMap<T>::createPipeline()
{
   m_graphicsPipeline = Graphics(
         m_logicalDevice,
         GraphicsPipelineType::PREFILTER_ENV_MAP,
         {
            m_dim,
            m_dim
         },
         m_renderPass,
         {
            {
               shaderType::VERTEX,
               "prefilterEnvMap"
            },
            {
               shaderType::FRAGMENT,
               "prefilterEnvMap"
            }
         },
         VK_SAMPLE_COUNT_1_BIT,
         // It uses the same attributes as the skybox shader.
         Attributes::SKYBOX::getBindingDescription(),
         Attributes::SKYBOX::getAttributeDescriptions(),
         {},
         {},
         GRAPHICS_PIPELINE::PREFILTER_ENV_MAP::SAMPLERS_INFO,
         {
            {
               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
               0,
               sizeof(PushBlockPrefilterEnv)
            }
         }
   );
}

template<typename T>
void PrefilteredEnvMap<T>::createOffscreenFramebuffer(
      const VkPhysicalDevice& physicalDevice,
      const VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool
) {

   m_offscreenImage = Image(
         physicalDevice,
         m_logicalDevice,
         m_dim,
         m_dim,
         m_format,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         false,
         1,
         VK_SAMPLE_COUNT_1_BIT,
         VK_IMAGE_ASPECT_COLOR_BIT,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY
   );

   std::vector<VkImageView> attachments = {m_offscreenImage.getImageView()};

   framebufferManager::createFramebuffer(
         m_logicalDevice,
         m_renderPass.get(),
         attachments,
         m_dim,
         m_dim,
         1,
         m_framebuffer
   );

   imageManager::transitionImageLayout(
         m_format,
         1,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         false,
         commandPool,
         graphicsQueue,
         m_offscreenImage.get()
   );
   
}

template<typename T>
void PrefilteredEnvMap<T>::createRenderPass()
{
   // Color Attachment
   VkAttachmentDescription colorAttachment{};
   attachmentUtils::createAttachmentDescriptionWithStencil(
         m_format,
         VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_CLEAR,
         VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
         VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         colorAttachment
   );

   // Attachment references
   
   VkAttachmentReference colorAttachmentRef{};
   attachmentUtils::createAttachmentReference(
         0,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
         colorAttachmentRef
   );

   // Subpasses
   VkSubpassDescription subPassDescript{};
   subPassUtils::createSubPassDescription(
         VK_PIPELINE_BIND_POINT_GRAPHICS,
         &colorAttachmentRef,
         nullptr,
         nullptr,
         subPassDescript
   );

   // Subpass dependencies
   std::vector<VkSubpassDependency> dependencies(2);
   subPassUtils::createSubPassDependency(
         // -Source parameters.
         //VK_SUBPASS_EXTERNAL means anything outside of a given render pass
         //scope. When used for srcSubpass it specifies anything that happened 
         //before the render pass. 
         VK_SUBPASS_EXTERNAL,
         // Operations that the subpass needs to wait on. 
         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
         VK_ACCESS_MEMORY_READ_BIT,
         // -Destination parameters.
         0,
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         (
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
         ),
         VK_DEPENDENCY_BY_REGION_BIT,
         dependencies[0]
   );
   subPassUtils::createSubPassDependency(
         0,
         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
         (
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
         ),
         // -Destination parameters.
         VK_SUBPASS_EXTERNAL,
         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
         VK_ACCESS_MEMORY_READ_BIT,
         VK_DEPENDENCY_BY_REGION_BIT,
         dependencies[1]
   );

   m_renderPass = RenderPass(
         m_logicalDevice,
         {colorAttachment},
         {subPassDescript},
         dependencies
   );
 
}

template<typename T>
void PrefilteredEnvMap<T>::createTargetImage(
      const VkPhysicalDevice& physicalDevice
) {

   m_targetImage = Image(
         physicalDevice,
         m_logicalDevice,
         m_dim,
         m_dim,
         m_format,
         VK_IMAGE_TILING_OPTIMAL,
         VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         true,
         floor(log2(m_dim)) + 1,
         VK_SAMPLE_COUNT_1_BIT,
         VK_IMAGE_ASPECT_COLOR_BIT,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_COMPONENT_SWIZZLE_IDENTITY,
         VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
         VK_FILTER_LINEAR
   );

}

template<typename T>
PrefilteredEnvMap<T>::~PrefilteredEnvMap() {}

template<typename T>
void PrefilteredEnvMap<T>::destroy()
{
   m_graphicsPipeline.destroy();
   m_descriptorPool.destroy();
   m_targetImage.destroy();
   m_offscreenImage.destroy();
   m_renderPass.destroy();

   vkDestroyFramebuffer(m_logicalDevice, m_framebuffer, nullptr);
}

template<typename T>
const Image& PrefilteredEnvMap<T>::get() const
{
   return m_targetImage;
}


////////////////////////////////////INSTANCES//////////////////////////////////
template class PrefilteredEnvMap<Attributes::SKYBOX::Vertex>;
