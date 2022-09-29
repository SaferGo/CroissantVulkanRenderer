#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 view;
   mat4 proj;
   vec4 lightPositions[10];
   vec4 lightColors[10];
   int  lightsCount;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main()
{
   
   vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);
   vec3 c = vec3(texture(texSampler, inTexCoord));

   for (int i = 0; i < ubo.lightsCount; i++)
   {
      vec3 lightDir = normalize(
            vec3(ubo.lightPositions[i]) - inPosition
      );

      float diffuse = max(
            0.0,
            dot(
               normalize(inNormal),
               lightDir
            )
      );

      finalColor += vec4(c * diffuse * vec3(ubo.lightColors[i]), 0.0);
   }

   outColor = clamp(finalColor, 0.0, 1.0);
}
