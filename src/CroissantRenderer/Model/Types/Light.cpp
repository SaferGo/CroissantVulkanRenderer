#include <CroissantRenderer/Model/Types/Light.h>

#include <CroissantRenderer/Settings/graphicsPipelineConfig.h>
#include <CroissantRenderer/Descriptor/Types/DescriptorTypes.h>
#include <CroissantRenderer/Descriptor/Types/UBO/UBOutils.h>
#include <CroissantRenderer/Buffer/bufferManager.h>
#include <CroissantRenderer/Math/mathUtils.h>
#include <CroissantRenderer/Texture/Type/NormalTexture.h>
#include <CroissantRenderer/Command/commandManager.h>

Light::Light(const ModelInfo& modelInfo)
   : Model(
         modelInfo.name,
         modelInfo.folderName,
         ModelType::LIGHT,
         glm::fvec4(modelInfo.pos, 1.0f),
         modelInfo.rot,
         modelInfo.size
    ),
    m_targetPos(glm::fvec4(modelInfo.endPos, 1.0f)),
    m_color(glm::fvec4(modelInfo.color, 1.0f)),
    m_lightType(modelInfo.lType)
{
   if (modelInfo.lType == LightType::DIRECTIONAL_LIGHT)
      m_intensity = 3.0f;
   else
      m_intensity = 70.0f;

   loadModel(
         (
            std::string(MODEL_DIR) +
            modelInfo.folderName + "/" +
            modelInfo.fileName
         ).c_str()
   );

   m_rot = glm::fvec3(0.0f);
}

Light::~Light() {}

void Light::destroy(const VkDevice& logicalDevice)
{
   m_ubo->destroy();

   for (auto& texture : m_texturesLoaded)
      texture->destroy();

   for (auto& mesh : m_meshes)
   {
      bufferManager::destroyBuffer(
            logicalDevice,
            mesh.vertexBuffer
      );
      bufferManager::destroyBuffer(
            logicalDevice,
            mesh.indexBuffer
      );
      
      bufferManager::freeMemory(
            logicalDevice,
            mesh.vertexMemory
      );
      bufferManager::freeMemory(
            logicalDevice,
            mesh.indexMemory
      );
   }
}

void Light::processMesh(aiMesh* mesh, const aiScene* scene)
{
   Mesh<Attributes::LIGHT::Vertex> newMesh;

   for (size_t i = 0; i < mesh->mNumVertices; i++)
   {
      Attributes::LIGHT::Vertex vertex{};

      vertex.pos = {
         mesh->mVertices[i].x,
         mesh->mVertices[i].y,
         mesh->mVertices[i].z
      };

      vertex.texCoord = {
         mesh->mTextureCoords[0][i].x,
         mesh->mTextureCoords[0][i].y,
      };

      newMesh.vertices.push_back(vertex);

   }

   for (size_t i = 0; i < mesh->mNumFaces; i++)
   {
      auto face = mesh->mFaces[i];
      for (size_t j = 0; j < face.mNumIndices; j++)
         newMesh.indices.push_back(face.mIndices[j]);
   }

   m_meshes.push_back(newMesh);
}

void Light::createUniformBuffers(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t& uboCount
) {
   m_ubo = std::make_shared<UBO>(
         physicalDevice,
         logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::Light)
   );
}
void Light::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorSetInfo* info,
      DescriptorPool& descriptorPool
) {
   std::vector<UBO*> opUBOs = {m_ubo.get()};

   for (auto& mesh : m_meshes)
   {
      mesh.descriptorSets = DescriptorSets(
            logicalDevice,
            GRAPHICS_PIPELINE::LIGHT::UBOS_INFO,
            GRAPHICS_PIPELINE::LIGHT::SAMPLERS_INFO,
            mesh.textures,
            descriptorSetLayout,
            descriptorPool,
            nullptr,
            opUBOs
      );
   }
}

void Light::bindData(
      const Graphics* graphicsPipeline,
      const VkCommandBuffer& commandBuffer,
      const uint32_t currentFrame
) {
   for (auto& mesh : m_meshes)
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

      commandManager::state::bindDescriptorSets(
            graphicsPipeline->getPipelineLayout(),
            PipelineType::GRAPHICS,
            // Index of first descriptor set.
            0,
            {mesh.descriptorSets.get(currentFrame)},
            // Dynamic offsets.
            {},
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
}

void Light::uploadVertexData(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkQueue& graphicsQueue,
      const std::shared_ptr<CommandPool>& commandPool
) {

   for (auto& mesh : m_meshes)
   {
      // Vertex Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            commandPool,
            physicalDevice,
            logicalDevice,
            mesh.vertices.data(),
            sizeof(mesh.vertices[0]) * mesh.vertices.size(),
            graphicsQueue,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            mesh.vertexMemory,
            mesh.vertexBuffer
         );

      // Index Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            commandPool,
            physicalDevice,
            logicalDevice,
            mesh.indices.data(),
            sizeof(mesh.indices[0]) * mesh.indices.size(),
            graphicsQueue,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            mesh.indexMemory,
            mesh.indexBuffer
      );
   }
}

void Light::uploadTextures(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkSampleCountFlagBits& samplesCount,
      const std::shared_ptr<CommandPool>& commandPool,
      const VkQueue& graphicsQueue
) {
   const size_t nTextures = GRAPHICS_PIPELINE::LIGHT::TEXTURES_PER_MESH_COUNT;
   const TextureToLoadInfo info = {
      "baseColor.png",
      "defaultTextures",
      VK_FORMAT_R8G8B8A8_SRGB,
      // channels
      4
   };

   for (auto& mesh : m_meshes)
   {
      for (size_t i = 0; i < nTextures; i++)
      {
         auto it = m_texturesID.find(info.name);

         if (it == m_texturesID.end())
         {
            mesh.textures.push_back(
                  std::make_shared<NormalTexture>(
                     physicalDevice,
                     logicalDevice,
                     info,
                     samplesCount,
                     commandPool,
                     graphicsQueue,
                     UsageType::TO_COLOR
                  )
            );

            m_texturesLoaded.push_back(mesh.textures[i]);
            m_texturesID[info.name] = m_texturesLoaded.size() - 1;
         } else
            mesh.textures.push_back(m_texturesLoaded[it->second]);
      }
   }
}

void Light::updateUBO(
      const VkDevice& logicalDevice,
      const uint32_t& currentFrame,
      const UBOinfo& uboInfo
) {

   m_dataInShader.model = mathUtils::getUpdatedModelMatrix(
         m_pos,
         m_rot,
         m_size
   );
   m_dataInShader.view = uboInfo.view;
   m_dataInShader.proj = uboInfo.proj;

   m_dataInShader.lightColor = m_color;

   size_t size = sizeof(m_dataInShader);
   UBOutils::updateUBO(
         logicalDevice,
         m_ubo,
         size,
         &m_dataInShader,
         currentFrame
   );
}

const glm::fvec4& Light::getColor() const
{
   return m_color;
}

const float& Light::getIntensity() const
{
   return m_intensity;
}

const glm::fvec4& Light::getTargetPos() const
{
   return m_targetPos;
}

const LightType& Light::getLightType() const
{
   return m_lightType;
}

void Light::setColor(const glm::fvec4& newColor)
{
   m_color = newColor;
}

void Light::setTargetPos(const glm::fvec4& pos)
{
   m_targetPos = pos;
}

void Light::setIntensity(const float& intensity)
{
   m_intensity = intensity;
}

