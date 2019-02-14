#version 450 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 viewRay;

layout(binding = 0) uniform sampler2D albedoTexture;
layout(binding = 1) uniform sampler2D normalsTexture;
layout(binding = 2) uniform sampler2D linearDepthTexture;
layout(binding = 3) uniform sampler2D ssaoNoiseTexture;
layout(binding = 4) uniform sampler2D ssaoTexture;

uniform mat4 invProj;
uniform vec2 noiseScale;
uniform int enableSSAO;
uniform int ssaoOnly;

// Returns the world position given the non-linearized depth value
/*vec3 worldPosition(float depth)
{
	//float zN = 2.0 * depth - 1.0;				// No need to convert from [0,1] to [-1,1] because we're using glClipControl to change it to [0,1] instead of the default [-1,1]
	vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, depth, 1.0);
	vec4 viewSpacePos = invProj * clipSpacePos;
	viewSpacePos /= viewSpacePos.w;
	vec4 worldSpacePos = invView * viewSpacePos;

	return worldSpacePos.xyz;
}*/

// Returns the view space position given the non-linearized depth value
vec3 viewSpacePosition(float depth)
{
	//float zN = 2.0 * depth - 1.0;				// No need to convert from [0,1] to [-1,1] because we're using glClipControl to change it to [0,1] instead of the default [-1,1]
	vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, depth, 1.0);
	vec4 viewSpacePos = invProj * clipSpacePos;
	viewSpacePos /= viewSpacePos.w;
	
	return viewSpacePos.xyz;
}

void main()
{
	float depth = texture(linearDepthTexture, uv).r;
	vec3 viewSpacePos = viewRay * depth;

	outColor.a=1.0;
	
	float ao = texture(ssaoTexture, uv).r;
	
	vec3 albedo = texture(albedoTexture, uv).rgb;
	if(enableSSAO > 0)
		albedo *= ao;
	
	albedo = pow(albedo, vec3(0.45));
	
	outColor.rgb = vec3(albedo);
	
	if(ssaoOnly > 0)
		outColor.rgb = vec3(ao);
}