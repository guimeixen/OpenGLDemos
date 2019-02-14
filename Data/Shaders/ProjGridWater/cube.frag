#version 450 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 normal;

uniform vec3 sunDir;
uniform vec3 sunLightColor;

void main()
{
	vec3 N = normalize(normal);
	
	vec3 diffuse = max(dot(N, sunDir), 0.0) * sunLightColor;
	
	outColor.rgb = diffuse;
	outColor.a = 1.0;
}