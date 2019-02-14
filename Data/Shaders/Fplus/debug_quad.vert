#version 450

layout(location = 0) in vec4 posuv;

layout(location = 0) out vec2 uv;

uniform vec2 transScale;

void main()
{
	uv = posuv.zw;
	gl_Position =vec4(posuv.x * transScale.y + transScale.x, posuv.y * transScale.y + transScale.x, 0.0, 1.0);
}