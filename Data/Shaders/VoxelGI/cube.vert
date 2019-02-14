#version 450 core

layout (location = 0) in vec3 inPos;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 color;

//layout(binding = 3, rgba8) uniform readonly image3D voxelTexture;
layout(binding = 3) uniform sampler3D voxelTexture;

struct Voxel
{
	vec3 pos;
	float pad;
};

layout(binding = 1, std140) readonly buffer VoxelSSBO
{
	Voxel voxels[];
};

uniform float voxelGridSize;
uniform float volumeSize;

void main()
{
	vec3 gridPos = voxels[gl_InstanceID].pos;

	float scale = voxelGridSize / volumeSize;		// Grid size = 64, voxel size = 64 then scale = 1
	float halfSize = voxelGridSize / 2.0;
	
	mat4 m = mat4(scale);
	m[3] = vec4(gridPos * scale - halfSize, 1.0);
	
	//color = imageLoad(voxelTexture, ivec3(gridPos)).rgb;
	color = textureLod(voxelTexture, gridPos / volumeSize, 0).rgb;
	
	gl_Position = projectionMatrix * viewMatrix * m * vec4(inPos, 1.0);
}