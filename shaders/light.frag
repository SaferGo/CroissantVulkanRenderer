#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 view;
   mat4 proj;
   vec4 lightColor;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main()
{
   outColor = vec4(
         texture(texSampler, inTexCoord).rgb * ubo.lightColor.rgb,
         1.0
   );
}
