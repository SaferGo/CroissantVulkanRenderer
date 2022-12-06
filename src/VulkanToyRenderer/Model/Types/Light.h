#pragma once

#include <VulkanToyRenderer/Model/Model.h>
#include <VulkanToyRenderer/Model/ModelInfo.h>
#include <VulkanToyRenderer/Features/ShadowMap.h>

#include <GLFW/glfw3.h>

class Light : public Model
{

public:

   Light(const ModelInfo& modelInfo);
   ~Light() override;
   void destroy(const VkDevice& logicalDevice) override;
   void createDescriptorSets(
         const VkDevice& logicalDevice,
         const VkDescriptorSetLayout& descriptorSetLayout,
         DescriptorSetInfo* info,
         DescriptorPool& descriptorPool
   ) override;
   void bindData(
         const Graphics* graphicsPipeline,
         const VkCommandBuffer& commandBuffer,
         const uint32_t currentFrame
   ) override;
   void updateUBO(
         const VkDevice& logicalDevice,
         const uint32_t& currentFrame,
         const UBOinfo& uboInfo
   ) override;

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
         

private:

   void createUniformBuffers(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const uint32_t& uboCount
   ) override;
   void uploadVertexData(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkQueue& graphicsQueue,
         const std::shared_ptr<CommandPool>& commandPool
   ) override;
   void uploadTextures(
         const VkPhysicalDevice& physicalDevice,
         const VkDevice& logicalDevice,
         const VkSampleCountFlagBits& samplesCount,
         const std::shared_ptr<CommandPool>& commandPool,
         const VkQueue& graphicsQueue
   ) override;
   void processMesh(aiMesh* mesh, const aiScene* scene) override;

   // To get the direction of directional and spot lights(m_endPos - m_Pos);
   glm::fvec4 m_targetPos;
   glm::fvec4 m_color;
   float      m_attenuation;
   float      m_radius;
   float      m_intensity;
   LightType  m_lightType;

   std::vector<Mesh<Attributes::LIGHT::Vertex>> m_meshes;
   DescriptorTypes::UniformBufferObject::Light m_dataInShader;
};
