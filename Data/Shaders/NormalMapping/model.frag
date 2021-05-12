#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 worldPos;
layout(location = 3) in mat3 TBN;

layout(binding = 0) uniform sampler2D diffuse;
layout(binding = 1) uniform sampler2D normalMap;

uniform vec3 lightPos;

void main()
{
	//vec3 N = normalize(normal);
	vec3 N = texture(normalMap, uv).rgb * 2.0 - 1.0;
	N = normalize(TBN * N);
	vec3 toLight = lightPos - worldPos;
	vec3 L = normalize(toLight);
	
	float diff = max(dot(N,L), 0.0);
	
	float dist = length(toLight);
	float att = clamp(1.0 - dist / 8.0, 0.0, 1.0);
	att *= att;
	diff *= att;

	vec3 diffuse = texture(diffuse, uv).rgb;
	diffuse *= diff;

	color.rgb = pow(diffuse, vec3(0.45));
	color.a = 1.0;
}