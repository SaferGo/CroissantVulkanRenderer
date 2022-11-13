#include <VulkanToyRenderer/Model/Types/Light.h>

#include <VulkanToyRenderer/Settings/config.h>
#include <VulkanToyRenderer/Descriptors/Types/DescriptorTypes.h>
#include <VulkanToyRenderer/Descriptors/Types/UBO/UBOutils.h>
#include <VulkanToyRenderer/Settings/graphicsPipelineConfig.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>

Light::Light(
      const std::string& name,
      const std::string& modelFileName,
      const LightType& lightType,
      const glm::fvec4& lightColor,
      const glm::fvec4& pos,
      const glm::fvec4& targetPos,
      const glm::fvec3& rot,
      const glm::fvec3& size,
      const float attenuation,
      const float radius
) : Model(name, ModelType::LIGHT, pos, rot, size),
    m_targetPos(targetPos),
    m_color(lightColor),
    m_lightType(lightType),
    m_attenuation(attenuation),
    m_radius(radius)
{
   if (lightType == LightType::DIRECTIONAL_LIGHT)
      m_intensity = 5.0f;
   else
      m_intensity = 70.0f;

   loadModel((std::string(MODEL_DIR) + modelFileName).c_str());

   m_rot = glm::fvec3(0.0f);
}

Light::~Light() {}

void Light::destroy(const VkDevice& logicalDevice)
{
   m_ubo.destroyUniformBuffersAndMemories(logicalDevice);

   for (auto& texture : m_texturesLoaded)
      texture->destroyTexture(logicalDevice);

   for (auto& mesh : m_meshes)
   {
      bufferManager::destroyBuffer(
            logicalDevice,
            mesh.m_vertexBuffer
      );
      bufferManager::destroyBuffer(
            logicalDevice,
            mesh.m_indexBuffer
      );
      
      bufferManager::freeMemory(
            logicalDevice,
            mesh.m_vertexMemory
      );
      bufferManager::freeMemory(
            logicalDevice,
            mesh.m_indexMemory
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

      vertex.normal = {
         mesh->mNormals[i].x,
         mesh->mNormals[i].y,
         mesh->mNormals[i].z
      };
      vertex.normal = glm::normalize(vertex.normal);

      vertex.texCoord = {
         mesh->mTextureCoords[0][i].x,
         mesh->mTextureCoords[0][i].y,
      };

      newMesh.m_vertices.push_back(vertex);

   }

   for (size_t i = 0; i < mesh->mNumFaces; i++)
   {
      auto face = mesh->mFaces[i];
      for (size_t j = 0; j < face.mNumIndices; j++)
         newMesh.m_indices.push_back(face.mIndices[j]);
   }

   m_meshes.push_back(newMesh);
}

void Light::createUniformBuffers(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t& uboCount
) {
   m_ubo.createUniformBuffers(
         physicalDevice,
         logicalDevice,
         uboCount,
         sizeof(DescriptorTypes::UniformBufferObject::Light)
   );
}

void Light::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      const ShadowMap* shadowMap,
      DescriptorPool& descriptorPool
) {
   std::vector<UBO*> opUBOs = {&m_ubo};

   for (auto& mesh : m_meshes)
   {
      mesh.m_descriptorSets = DescriptorSets(
            logicalDevice,
            GRAPHICS_PIPELINE::LIGHT::UBOS_INFO,
            GRAPHICS_PIPELINE::LIGHT::SAMPLERS_INFO,
            mesh.m_textures,
            nullptr,
            nullptr,
            opUBOs,
            descriptorSetLayout,
            descriptorPool
      );
   }
}

void Light::uploadVertexData(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      VkQueue& graphicsQueue,
      CommandPool& commandPool
) {

   for (auto& mesh : m_meshes)
   {
      // Vertex Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            commandPool,
            physicalDevice,
            logicalDevice,
            mesh.m_vertices.data(),
            sizeof(mesh.m_vertices[0]) * mesh.m_vertices.size(),
            graphicsQueue,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            mesh.m_vertexMemory,
            mesh.m_vertexBuffer
         );

      // Index Buffer(with staging buffer)
      bufferManager::createBufferAndTransferToDevice(
            commandPool,
            physicalDevice,
            logicalDevice,
            mesh.m_indices.data(),
            sizeof(mesh.m_indices[0]) * mesh.m_indices.size(),
            graphicsQueue,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            mesh.m_indexMemory,
            mesh.m_indexBuffer
      );
   }
}

void Light::createTextures(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const VkSampleCountFlagBits& samplesCount,
      CommandPool& commandPool,
      VkQueue& graphicsQueue
) {
   const size_t nTextures = GRAPHICS_PIPELINE::LIGHT::TEXTURES_PER_MESH_COUNT;
   const TextureToLoadInfo info = {
      "textures/default.png",
      VK_FORMAT_R8G8B8A8_SRGB
   };

   for (auto& mesh : m_meshes)
   {
      for (size_t i = 0; i < nTextures; i++)
      {
         auto it = (
               m_texturesID.find(info.name)
         );

         if (it == m_texturesID.end())
         {
            mesh.m_textures.push_back(
                  std::make_shared<Texture>(
                     physicalDevice,
                     logicalDevice,
                     info,
                     // isSkybox
                     false,
                     samplesCount,
                     commandPool,
                     graphicsQueue
                  )
            );

            m_texturesLoaded.push_back(mesh.m_textures[i]);
            m_texturesID[info.name] = (
                  m_texturesLoaded.size() - 1
            );
         } else
            mesh.m_textures.push_back(m_texturesLoaded[it->second]);

      }
   }
}

void Light::updateUBO(
      const VkDevice& logicalDevice,
      const glm::vec4& cameraPos,
      const glm::mat4& view,
      const glm::mat4& proj,
      const uint32_t& currentFrame
) {

   m_dataInShader.model = UBOutils::getUpdatedModelMatrix(
         m_pos,
         m_rot,
         m_size
   );
   m_dataInShader.view = view;
   m_dataInShader.proj = proj;

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

const float& Light::getAttenuation() const
{
   return m_attenuation;
}

const float& Light::getRadius() const
{
   return m_radius;
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

void Light::setAttenuation(const float& attenuation)
{
   m_attenuation = attenuation;
}

void Light::setIntensity(const float& intensity)
{
   m_intensity = intensity;
}

void Light::setRadius(const float& radius)
{
   m_radius = radius;
}
