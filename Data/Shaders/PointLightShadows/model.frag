#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in vec2 uv;

layout(binding = 0) uniform sampler2D diffuseTex;
layout(binding = 1) uniform samplerCubeShadow shadowMap;

uniform vec3 lightPos;
uniform float lightRadius;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform float farPlane;

void main()
{
	vec3 N = normalize(normal);
	vec3 toLight = lightPos - worldPos;
	vec3 L = normalize(toLight);
	
	vec3 diff = max(dot(N,L), 0.0) * lightColor * lightIntensity;
	
	float dist = length(toLight);
	float att = clamp(1.0 - dist / lightRadius, 0.0, 1.0);
	att *= att;
	diff *= att;
	
	/*float closestDepth = texture(shadowMap, vec4(-toLight, 0.05).r;
	closestDepth *= farPlane;		// far plane
	float currentDepth = dist;
	float shadow = currentDepth - 0.05 > closestDepth ? 0.0 : 1.0;*/
	
	float shadow = texture(shadowMap, vec4(-toLight, (dist - 0.05) / farPlane)).r;
	
	vec3 result = (vec3(0.15) + diff * shadow)/* * texture(diffuseTex, uv).rgb*/;
	
	outColor = vec4(result, 1.0);
	//outColor = vec4(vec3(closestDepth / 25.0), 1.0);
}