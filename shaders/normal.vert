#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 view;
   mat4 proj;
   vec4 lightPositions[10];
   vec4 lightColors[10];
   vec4 cameraPos;
   int  lightsCount;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outTexCoord;
layout(location = 3) out vec3 outNormal;

void main()
{
   gl_Position = (
         ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0)
   );

   outPosition = vec3(ubo.model * vec4(inPosition, 1.0));
   outColor = inColor;
   outTexCoord = inTexCoord;
   outNormal = normalize(
         vec3(
            transpose(inverse(ubo.model)) * vec4(inNormal, 0.0)
         )
   );
}
