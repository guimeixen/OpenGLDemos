#version 450

layout(location = 0) in vec3 inPos;

uniform mat4 modelMatrix;

void main()
{
	gl_Position = modelMatrix * vec4(inPos, 1.0);
}