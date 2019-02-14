#version 450 core

layout(location = 0) in vec4 posuv;

uniform mat4 modelMatrix;
uniform mat4 proj;
uniform mat4 view;

void main()
{
	gl_Position = proj * view * modelMatrix * vec4(posuv.xy * 0.5, 0.0, 1.0);
}