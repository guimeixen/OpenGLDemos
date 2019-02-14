#version 450 core

layout(location = 0) in vec3 inPos;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec3 normal;

uniform mat4 projView;
uniform mat4 modelMatrix;

void main()
{
	normal = inNormal;
	gl_Position = projView * modelMatrix * vec4(inPos, 1.0);
}