#version 450 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;
layout(location = 2) out float viewSpaceDepth;
layout(location = 3) out vec3 worldPos;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main()
{
	uv = inUv;
	normal = inNormal;
	
	vec4 wPos = modelMatrix * vec4(inPos, 1.0);
	worldPos = wPos.xyz;
	
	vec4 posViewSpace = viewMatrix * wPos;
	gl_Position = projectionMatrix * posViewSpace;
	
	viewSpaceDepth = posViewSpace.z;
}