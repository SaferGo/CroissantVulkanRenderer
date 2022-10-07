#include <VulkanToyRenderer/Model/Model.h>

#include <iostream>
#include <limits>
#include <vector>
#include <unordered_map>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <VulkanToyRenderer/Model/Vertex.h>
#include <VulkanToyRenderer/Buffers/bufferManager.h>
#include <VulkanToyRenderer/Descriptors/DescriptorTypes/DescriptorTypes.h>

Model::Model(
      const char* pathToMesh,
      const std::string& texture,
      const std::string& modelName,
      const bool lightModel,
      const glm::fvec4& lightColor
) : m_name(modelName),
   m_textureFileName(texture),
   m_isLightModel(lightModel),
   lightColor(lightColor)
{
   // We'll read the texture file later(after the creation of a cmd pool and
   // the queue handles).
   // Improve this.
   m_textureFileName= texture;
   loadVertexInfo(pathToMesh);
}

template<typename T>
void Model::updateUBO(
      const VkDevice& logicalDevice,
      T& newUbo,
      const uint32_t& currentFrame
) {
   void* data;
   vkMapMemory(
         logicalDevice,
         m_ubo.getUniformBufferMemory(currentFrame),
         0,
         sizeof(newUbo),
         0,
         &data
   );
      memcpy(data, &newUbo, sizeof(newUbo));
   vkUnmapMemory(
         logicalDevice,
         m_ubo.getUniformBufferMemory(currentFrame)
   );
}
//////////////////////////////////Instances////////////////////////////////////
template void Model::updateUBO<DescriptorTypes::UniformBufferObject::Normal>(
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject::Normal& newUbo,
      const uint32_t& currentFrame
);
template void Model::updateUBO<DescriptorTypes::UniformBufferObject::Light>(
      const VkDevice& logicalDevice,
      DescriptorTypes::UniformBufferObject::Light& newUbo,
      const uint32_t& currentFrame
);
///////////////////////////////////////////////////////////////////////////////

Model::~Model() {}

void Model::destroy(const VkDevice& logicalDevice)
{
   m_ubo.destroyUniformBuffersAndMemories(logicalDevice);
   m_texture->destroyTexture(logicalDevice);
   bufferManager::destroyBuffer(
         logicalDevice,
         m_vertexBuffer
   );
   bufferManager::destroyBuffer(
         logicalDevice,
         m_indexBuffer
   );
   
   bufferManager::freeMemory(
         logicalDevice,
         m_vertexMemory
   );
   bufferManager::freeMemory(
         logicalDevice,
         m_indexMemory
   );
}


const std::string& Model::getName() const
{
   return m_name;
}

void Model::loadMaterial(aiMaterial* material)
{
   aiString materialName;
   material->Get(AI_MATKEY_NAME, materialName);
   std::string mName = materialName.C_Str();

   if (mName != "DefaultMaterial")
   {
      aiColor3D color;
      material->Get(AI_MATKEY_COLOR_AMBIENT, color);
      materials.ambient = glm::fvec4(color.r, color.g, color.b, 1.0f);
      material->Get(AI_MATKEY_COLOR_DIFFUSE, color);              
      materials.diffuse = glm::fvec4(color.r, color.g, color.b, 1.0f);
      material->Get(AI_MATKEY_COLOR_SPECULAR, color); 
      materials.specular = glm::fvec4(color.r, color.g, color.b, 1.0f);
      material->Get(AI_MATKEY_SHININESS, materials.shininess);    
   } else
   {
      materials.ambient = glm::fvec4(0.0003);
      materials.diffuse = glm::fvec4(0.5);
      materials.specular = glm::fvec4(0.5);
      materials.shininess = 33;
   }
   
}

void Model::loadVertexInfo(const char* pathToMesh)
{
   Assimp::Importer importer;
   const auto* scene = importer.ReadFile(
         pathToMesh,
         aiProcess_Triangulate | aiProcess_FlipUVs
   );

   if (!scene ||
       scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
       !scene->mRootNode
   ) {
      throw std::runtime_error(
            "ERROR::ASSIMP" +
            std::string(importer.GetErrorString())
      );
   }

   ///
   aiNode* node = scene->mRootNode->mChildren[0];
   auto* mesh = scene->mMeshes[node->mMeshes[0]];

   initExtremeValues();

   for (size_t i = 0; i < mesh->mNumVertices; i++)
   {
      Vertex vertex{};

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

      vertex.texCoord = {
         mesh->mTextureCoords[0][i].x,
         // Flips the vertical component.
         1.0f - mesh->mTextureCoords[0][i].y,
      };



      // In this case de mesh is already iluminated in the texture.
      vertex.color = {1.0f, 1.0f, 1.0f};

      m_vertices.push_back(vertex);

      // These will help us to move model to the center.
      extremeX[0] = std::fmin(vertex.pos.x, extremeX[0]);
      extremeX[1] = std::fmax(vertex.pos.x, extremeX[1]);
      extremeY[0] = std::fmin(vertex.pos.y, extremeY[0]);
      extremeY[1] = std::fmax(vertex.pos.y, extremeY[1]);
      extremeZ[0] = std::fmin(vertex.pos.z, extremeZ[0]);
      extremeZ[1] = std::fmax(vertex.pos.z, extremeZ[1]);
   }

   for (size_t i = 0; i < mesh->mNumFaces; i++)
   {
      auto face = mesh->mFaces[i];
      for (size_t j = 0; j < face.mNumIndices; j++)
         m_indices.push_back(face.mIndices[j]);
   }

   aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
   loadMaterial(material);

   makeBasicTransformations();
   
   actualPos = glm::fvec4(0.0f);
   actualSize = glm::fvec3(1.0f);
   actualRot = glm::fvec3(0.0f);
}

void Model::uploadVertexData(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      VkQueue& graphicsQueue,
      CommandPool& commandPool
) {

   // Vertex Buffer(with staging buffer)
   bufferManager::createBufferAndTransferToDevice(
         commandPool,
         physicalDevice,
         logicalDevice,
         m_vertices,
         graphicsQueue,
         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         m_vertexMemory,
         m_vertexBuffer
      );

   // Index Buffer(with staging buffer)
   bufferManager::createBufferAndTransferToDevice(
         commandPool,
         physicalDevice,
         logicalDevice,
         m_indices,
         graphicsQueue,
         VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         m_indexMemory,
         m_indexBuffer
   );

}

void Model::createTexture(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      CommandPool& commandPool,
      VkQueue& graphicsQueue,
      const VkFormat& format
) {
   m_texture = std::make_unique<Texture>(
         physicalDevice,
         logicalDevice,
         m_textureFileName,
         format,
         commandPool,
         graphicsQueue
   );
}

void Model::createUniformBuffers(
      const VkPhysicalDevice& physicalDevice,
      const VkDevice& logicalDevice,
      const uint32_t& uboCount
) {
   m_ubo.createUniformBuffers(
         physicalDevice,
         logicalDevice,
         uboCount,
         (m_isLightModel) ?
            sizeof(DescriptorTypes::UniformBufferObject::Light) :
            sizeof(DescriptorTypes::UniformBufferObject::Normal)
   );
}

void Model::createDescriptorSets(
      const VkDevice& logicalDevice,
      const VkDescriptorSetLayout& descriptorSetLayout,
      DescriptorPool& descriptorPool
) {
   m_descriptorSets.createDescriptorSets(
         logicalDevice,
         // List of all the descriptors
         // (improve this?)
         m_texture->getTextureImageView(),
         m_texture->getTextureSampler(),
         m_ubo.getUniformBuffers(),
         descriptorSetLayout,
         descriptorPool
   );
}

const VkDescriptorSet& Model::getDescriptorSet(const uint32_t index) const
{
   return m_descriptorSets.getDescriptorSet(index);
}

const bool Model::isLightModel() const
{
   return m_isLightModel;
}

VkBuffer& Model::getVertexBuffer()
{
   return m_vertexBuffer;
}

VkBuffer& Model::getIndexBuffer()
{
   return m_indexBuffer;
}

uint32_t Model::getIndexCount()
{
   return m_indices.size();
}

void Model::initExtremeValues()
{
   extremeX[0] = extremeY[0] = extremeZ[0] = std::numeric_limits<float>::max();
   extremeX[1] = extremeY[1] = extremeZ[1] = std::numeric_limits<float>::min();

}

/*
 * Translates and scales(depending if the model is too big) all the
 * vertices and normals of the model to the center.
 */
void Model::makeBasicTransformations()
{
   glm::mat4 model = glm::mat4(1.0f);

   float maxZ = std::fmax(
         std::fmax(
            std::fabs(extremeX[1]),
            std::fabs(extremeX[0])
         ),
         std::fmax(
            std::fabs(extremeY[1]),
            std::fabs(extremeY[0])
         )
   );
   maxZ = std::fmax(
         maxZ,
         std::fmax(
            std::fabs(extremeZ[1]),
            std::fabs(extremeZ[0])
         )
   );

   initExtremeValues();

   // If the mesh is too big, this will scale it.
   if (maxZ > 1.0f || maxZ < -1.0f)
   {
      model = glm::scale(
            model,
            glm::fvec3(1.0f / maxZ)
      );

      for (auto& vertex : m_vertices)
      {
         vertex.pos = glm::fvec3(model * glm::fvec4(vertex.pos, 1.0f));
         vertex.normal = glm::fvec3(
               glm::transpose(glm::inverse(model)) * 
               glm::vec4(vertex.normal, 0.0)
         );

         extremeX[0] = std::fmin(vertex.pos.x, extremeX[0]);
         extremeX[1] = std::fmax(vertex.pos.x, extremeX[1]);
         extremeY[0] = std::fmin(vertex.pos.y, extremeY[0]);
         extremeY[1] = std::fmax(vertex.pos.y, extremeY[1]);
         extremeZ[0] = std::fmin(vertex.pos.z, extremeZ[0]);
         extremeZ[1] = std::fmax(vertex.pos.z, extremeZ[1]);

      }
   }


   model = glm::translate(
         glm::mat4(1.0f),
         glm::fvec3(
            -((extremeX[1] + extremeX[0]) / 2.0),
            -((extremeY[1] + extremeY[0]) / 2.0),
            -((extremeZ[1] + extremeZ[0]) / 2.0)
         )
   );

   initExtremeValues();

   for (auto& vertex : m_vertices)
   {
      vertex.pos = glm::fvec3(model * glm::fvec4(vertex.pos, 1.0f));
      vertex.normal = glm::fvec3(
            glm::transpose(glm::inverse(model)) * glm::vec4(vertex.normal, 0.0)
      );

      extremeX[0] = std::fmin(vertex.pos.x, extremeX[0]);
      extremeX[1] = std::fmax(vertex.pos.x, extremeX[1]);
      extremeY[0] = std::fmin(vertex.pos.y, extremeY[0]);
      extremeY[1] = std::fmax(vertex.pos.y, extremeY[1]);
      extremeZ[0] = std::fmin(vertex.pos.z, extremeZ[0]);
      extremeZ[1] = std::fmax(vertex.pos.z, extremeZ[1]);

   }
}
