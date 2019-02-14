#version 450 core

in vec3 worldPos;
in vec3 normal;

layout(binding = 0, rgba8) uniform writeonly image3D voxelTexture;

bool isInsideCube(const vec3 p)
{
	return abs(p.x) < 1.0 && abs(p.y) < 1.0 && abs(p.z) < 1.0;
}

void main()
{
	vec3 color = vec3(0.0);
	
	if(!isInsideCube(worldPos))
		return;
		
	vec3 voxel = worldPos * 0.5 + 0.5;
	ivec3 dim = imageSize(voxelTexture);
	imageStore(voxelTexture, ivec3(dim * voxel), vec4(color, 1.0));
}