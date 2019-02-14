#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 worldPos;

uniform mat4 modelMatrix;
uniform mat4 proj;
uniform mat4 view;

void main()
{
	uv = inUv;
	normal = inNormal;
	vec4 wPos = modelMatrix * vec4(pos, 1.0);
	worldPos = wPos.xyz;
	gl_Position = proj * view * wPos;
}