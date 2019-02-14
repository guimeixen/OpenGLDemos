#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out vec2 uv;

uniform mat4 projView;
uniform mat4 modelMatrix;

void main()
{
	normal = inNormal;
	uv = inUv;
	vec4 wPos = modelMatrix * vec4(inPos, 1.0);
	worldPos = wPos.xyz;
	gl_Position = projView * wPos;
}