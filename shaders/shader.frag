#version 450

layout(binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 view;
   mat4 proj;
   vec3 lightPositions;
   vec3 lightColors;
   int  lightsCount;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inPosition;

layout(location = 0) out vec4 outColor;

void main()
{
   float diffuse = max(
         0.0,
         dot(normalize(inNormal), normalize(ubo.lightPositions - inPosition))
   );

   //for (int i = 0; i < ubo.lightsCount; i++)
   //{
   //   vec3 lightDir = normalize(
   //      ubo.lightPositions - inPosition
   //   );

   //   diffuse += max(
   //         0.0,
   //         dot(inNormal, lightDir)
   //   );
   //}

   //outColor = vec4(clamp(
   //      ubo.lightPositions * texture(texSampler, inTexCoord),
   //      0.0,
   //      1.0
   //), 1.0);
   outColor = vec4(diffuse * vec3(texture(texSampler, inTexCoord)), 1.0);
}
