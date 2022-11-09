#version 450

layout(std140, binding = 0) uniform UniformBufferObject
{
   mat4 model;
   mat4 lightSpace;
} ubo;

layout(location = 0) in vec3 inPosition;

void main()
{
   gl_Position = (
         ubo.lightSpace * ubo.model * vec4(inPosition, 1.0)
   );
}
