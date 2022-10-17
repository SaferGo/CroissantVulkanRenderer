#version 450

layout(binding = 1) uniform samplerCube cubemapSampler;

layout(location = 0) in vec3 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
   outColor = texture(cubemapSampler, inTexCoord);
}
