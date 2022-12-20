// Code from: https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/06b2dbf65793a0f912216cd63ab542fc44fde844/data/shaders/filtercube.vert

#version 450

layout (location = 0) in vec3 inPos;

layout(push_constant) uniform PushConsts {
	layout (offset = 0) mat4 mvp;
} pushConsts;

layout (location = 0) out vec3 outUVW;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() 
{
	gl_Position = pushConsts.mvp * vec4(inPos.xyz, 1.0);

	outUVW = inPos;
}
