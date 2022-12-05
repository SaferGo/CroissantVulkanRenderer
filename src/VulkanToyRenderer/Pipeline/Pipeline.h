#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

enum shaderType
{
   NONE = 0,
   VERTEX = 1,
   FRAGMENT = 2,
   COMPUTE = 3
};

struct ShaderInfo
{
   shaderType type;
   std::string fileName;

   ShaderInfo(const shaderType& sType, const std::string& fName) 
   :  type(sType), fileName(fName) {}
};

enum class PipelineType
{
   NONE = 0,
   GRAPHICS = 1,
   COMPUTE = 2,
};

class Pipeline
{

public:

   Pipeline();
   Pipeline(
         const VkDevice& logicalDevice,
         const PipelineType& type
   );
   virtual ~Pipeline() = 0;
   const VkPipeline& get() const;
   const VkPipelineLayout& getPipelineLayout() const;
   const PipelineType& getType() const;
   const VkDescriptorSetLayout& getDescriptorSetLayout() const;
   void destroy();

protected:

   virtual void createShaderStageInfo(
      const VkShaderModule& shaderModule,
      const shaderType& type,
      VkPipelineShaderStageCreateInfo& shaderStageInfo
   ) = 0;
   void createShaderModule(
      const ShaderInfo& shaderInfos,
      VkShaderModule& shaderModule
   );
   void createPipelineLayout(
         const VkDescriptorSetLayout& descriptorSetLayout
   );

   PipelineType            m_type;

   VkDevice                m_logicalDevice;

   VkPipeline              m_pipeline;
   VkPipelineLayout        m_pipelineLayout;

   VkDescriptorSetLayout   m_descriptorSetLayout;

};
