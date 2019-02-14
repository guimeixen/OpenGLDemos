#version 450 core

layout(location = 0) in vec4 posuv;
layout(location = 1) in vec4 inColor;

out vec2 uv;
out vec4 color;

uniform mat4 projectionMatrix;

void main()
{
	color = inColor;
	uv = vec2(posuv.z, posuv.w);
	gl_Position = projectionMatrix * vec4(posuv.xy, 0.0, 1.0);
}