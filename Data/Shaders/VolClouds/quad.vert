#version 450 core

layout(location = 0) in vec4 posuv;

layout (location = 0) out vec2 uv;

void main()
{
	uv = posuv.zw;
	gl_Position = vec4(posuv.xy, 0.0, 1.0);
}