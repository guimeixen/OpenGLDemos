#version 450 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec2 uv;

uniform mat4 projView;
uniform mat4 modelMatrix;

void main()
{
	uv = inUv;
	gl_Position = projView * modelMatrix * vec4(inPos, 1.0);
}

