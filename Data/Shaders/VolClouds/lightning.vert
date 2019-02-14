#version 450 core

layout (location = 0) in vec3 inPos;

uniform mat4 projView;
uniform mat4 modelMatrix;

void main()
{
	gl_Position = projView * modelMatrix * vec4(inPos, 1.0);
}