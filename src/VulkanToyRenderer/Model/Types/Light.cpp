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
      const glm::fvec3& rot,
      const glm::fvec3& size,
      const float attenuation,
      const float radius
) : Model(name, ModelType::LIGHT, pos, rot, size),
    m_color(lightColor),
    m_lightType(lightType),
    m_attenuation(attenuation),
    m_radius(radius)
{

   loadModel((std::string(MODEL_DIR) + modelFileName).c_str());

   for (auto& mesh : m_meshes)
   {
      mesh.m_textures.resize(
            GRAPHICS_PIPELINE::LIGHT::TEXTURES_PER_MESH_COUNT
      );
      mesh.m_texturesToLoadInfo.push_back(
            {
               "textures/default.png",
               VK_FORMAT_R8G8B8A8_SRGB
            }
      );
   }
   m_rot = glm::fvec3(0.0f);
}

Light::~Light() {}

void Light::destroy(const VkDevice& logicalDevice)
{
   m_ubo.destroyUniformBuffersAndMemories(logicalDevice);

   for (auto& mesh : m_meshes)
   {
      for (auto& texture : mesh.m_textures) 
         texture.destroyTexture(logicalDevice);

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
   for (auto& mesh : m_meshes)
   {
      for (size_t i = 0; i < mesh.m_textures.size(); i++)
      {
         mesh.m_textures[i] = Texture(
               physicalDevice,
               logicalDevice,
               mesh.m_texturesToLoadInfo[i],
               // isSkybox
               false,
               samplesCount,
               commandPool,
               graphicsQueue
         );
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

   DescriptorTypes::UniformBufferObject::Light newUBO;

   newUBO.model = UBOutils::getUpdatedModelMatrix(
         m_pos,
         m_rot,
         m_size
   );
   newUBO.view = view;
   newUBO.proj = proj;

   newUBO.lightColor = m_color;

   size_t size = sizeof(newUBO);
   UBOutils::updateUBO(logicalDevice, m_ubo, size, &newUBO, currentFrame);
}

const glm::fvec4& Light::getColor() const
{
   return m_color;
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

void Light::setAttenuation(const float& attenuation)
{
   m_attenuation = attenuation;
}

void Light::setRadius(const float& radius)
{
   m_radius = radius;
}
