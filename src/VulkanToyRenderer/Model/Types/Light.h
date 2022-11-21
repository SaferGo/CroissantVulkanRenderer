#pragma once

#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>

#include <GLFW/glfw3.h>

enum class LightType
{
   DIRECTIONAL_LIGHT = 0,
   POINT_LIGHT       = 1,
   SPOT_LIGHT        = 2,
   NONE              = 3
};

class Light : public Model
{

public:

   Light(
         const std::string& name,
         const std::string& modelFilename,
         const LightType& lightType,
         const glm::fvec4& lightColor,
         const glm::fvec4& pos = glm::fvec4(0.0f),
         const glm::fvec4& targetPos = glm::fvec4(1.0f),
         const glm::fvec3& rot = glm::fvec3(0.0f),
         const glm::fvec3& size = glm::fvec3(1.0f),
         const float attenuation = 1.0f,
         const float radius = 1.0f
   );
   ~Light() override;
   void destroy(const VkDevice& logicalDevice) override;
   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorPool& descriptorPool
   );
   void createUniformBuffers(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   ) override;
   void uploadVertexData(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool
   ) override;
   void loadTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkSampleCountFlagBits& samplesCount,
         const std::shared_ptr<CommandPool>& commandPool,
         VkQueue& graphicsQueue
   ) override;
   void updateUBO(
         const VkDevice& logicalDevice,
         const glm::vec4& cameraPos,
         const glm::mat4& view,
         const glm::mat4& proj,
         const uint32_t& currentFrame
   );

   const glm::fvec4& getColor() const;
   const glm::fvec4& getTargetPos() const;
   const float& getAttenuation() const;
   const float& getRadius() const;
   const float& getIntensity() const;
   const LightType& getLightType() const;
   void setColor(const glm::fvec4& newColor);
   void setAttenuation(const float& attenuation);
   void setIntensity(const float& intensity);
   void setRadius(const float& radius);
   void setTargetPos(const glm::fvec4& pos);
         
   // Info to update UBO.
   float extremeX[2];
   float extremeY[2];
   float extremeZ[2];

   std::vector<Mesh<Attributes::LIGHT::Vertex>> m_meshes;

private:

   void processMesh(aiMesh* mesh, const aiScene* scene) override;

   // To get the direction of directional and spot lights(m_endPos - m_Pos);
   glm::fvec4 m_targetPos;
   glm::fvec4 m_color;
   float      m_attenuation;
   float      m_radius;
   float      m_intensity;
   LightType  m_lightType;

   DescriptorTypes::UniformBufferObject::Light m_dataInShader;
};
