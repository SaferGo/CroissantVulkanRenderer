#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 view;
   mat4 proj;
   mat4 lightSpace;
   vec4 cameraPos;
   int  lightsCount;

} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;
layout(location = 5) in vec4 inShadowCoords;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;
layout(location = 4) out vec3 outBitangent;
layout(location = 5) out vec4 outShadowCoords;

const mat4 bias = mat4(
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.5, 0.5, 0.0, 1.0
);

void main()
{
   gl_Position = (
         ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0)
   );

   outPosition = vec3(ubo.model * vec4(inPosition, 1.0));
   outTexCoord = inTexCoord;

   outTangent   = normalize(mat3(ubo.model) * inTangent);
   outNormal    = normalize(mat3(ubo.model) * inNormal);

   // Gram-Schmidt -> reorthogonalization
   outTangent = normalize(outTangent - dot(outTangent, outNormal) * outNormal);

   outBitangent = cross(outNormal, outTangent);

   outShadowCoords = (bias * ubo.lightSpace * ubo.model) * vec4(inPosition, 1.0);
}
