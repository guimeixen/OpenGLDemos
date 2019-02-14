#version 450 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 worldPos;

uniform float lightIntensity;

void main()
{
	vec3 N = normalize(normal);
	
	vec3 L = normalize(vec3(0.0, -1.5, 0.0) - worldPos);
	
	//vec3 diffuse = max(dot(N, L), 0.0) * vec3(0.8, 0.7, 1.0) * lightIntensity * 3.0;
	vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0) * lightIntensity * 3.0;
	
	outColor = vec4(diffuse,1.0);
	//outColor.a=1.0;
}