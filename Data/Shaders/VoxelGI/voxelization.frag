#version 450 core

in vec3 normal;
in vec2 uv;
in vec4 lightSpacePos;
flat in int axis;

uniform float time;

layout(binding = 0) uniform sampler2D texDiffuse;
layout(binding = 3, rgba8) uniform writeonly image3D voxelTexture;
layout(binding = 4) uniform sampler2DShadow shadowMap;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 emissiveColor;

void main()
{
	ivec3 dim = imageSize(voxelTexture);

	vec3 N = normalize(normal);
	
	vec3 shadowUV = vec3(lightSpacePos.xy, lightSpacePos.z - 0.005);
	float shadow = texture(shadowMap, shadowUV).r;
	
	/*float closestDepth = texture(shadowMap, lightSpacePos.xy).r;
	float currentDepth = lightSpacePos.z;
	
	float bias = 0.005;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;*/
	
	vec3 tempPos = vec3(gl_FragCoord.xy, gl_FragCoord.z * dim.z);
	vec3 voxelPos;
	
	if (axis == 1)
	{
		voxelPos.x = dim.x - tempPos.z;
		voxelPos.z = dim.x - tempPos.x;			// Flip the z otherwise the scene is flipped on the z axis
		voxelPos.y = tempPos.y;
	}
	else if (axis == 2)
	{
		voxelPos.z = dim.y - tempPos.y;
		voxelPos.y = dim.y - tempPos.z;		// Flip the z
		voxelPos.x = tempPos.x;
	}
	else
	{
		voxelPos.z = dim.z - tempPos.z;
		voxelPos.x = tempPos.x;
		voxelPos.y = tempPos.y;
	}
	
	vec3 diffuse = max(dot(N, lightDir), 0.0) * lightColor;
	vec3 color = diffuse * shadow * texture(texDiffuse, uv).rgb;
	
	color += emissiveColor;

	imageStore(voxelTexture, ivec3(voxelPos), vec4(color, 1.0));
}
