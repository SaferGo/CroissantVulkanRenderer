#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 view;
   mat4 proj;
   vec4 lightColor;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;

void main()
{
   gl_Position = (
         ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0)
   );

   outPosition = inPosition;
   outTexCoord = inTexCoord;
   outNormal = ((transpose(inverse(ubo.model)) * vec4(inNormal, 0.0))).xyz;
}
