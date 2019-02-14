#version 450 core

layout(location = 0) out vec4 outAO;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 viewRay;

layout(binding = 1) uniform sampler2D normalsTexture;
layout(binding = 2) uniform sampler2D linearDepthTexture;
layout(binding = 3) uniform sampler2D ssaoNoiseTexture;

uniform mat4 invProj;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform vec2 noiseScale;
uniform vec3 samples[8];
uniform float radius;
uniform float intensity;

void main()
{
	vec3 viewSpacePos = viewRay * texture(linearDepthTexture, uv).r;
	vec3 normal = vec3((viewMatrix * vec4(texture(normalsTexture, uv).rgb, 0.0)).xyz);			// Normals needs to be in view space
	vec3 randomVec = texture(ssaoNoiseTexture, uv * noiseScale).rgb;
	
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);
	
	const float bias = 0.025;
	float occlusion = 0.0;
	
	for (int i = 0; i < 8; i++)
	{
		vec3 s = TBN * samples[i];		// rotate the sample points
		s = viewSpacePos + s * radius;
		
		vec4 offset = vec4(s, 1.0);
		offset = projectionMatrix * offset;		// From view space to clip space
		offset.xy /= offset.w;			// Perspective divide
		offset.xy = offset.xy * 0.5 + 0.5;
		
		float sampleDepth = texture(linearDepthTexture, offset.xy).r * -100.0;
		
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewSpacePos.z - sampleDepth));
		//occlusion += (sampleDepth >= s.z + bias ? 1.0 : 0.0) * rangeCheck;
		occlusion += step(s.z + bias, sampleDepth) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / 8.0);
	occlusion = pow(occlusion, intensity);
	outAO = vec4(occlusion,0.0,0.0,1.0);
}