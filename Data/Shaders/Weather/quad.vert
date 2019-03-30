#version 450 core

layout(location = 0) in vec4 posuv;

layout (location = 0) out vec2 uv;

uniform float depth;
uniform float flipY;

void main()
{
	uv = posuv.zw;
	uv.y = flipY > 0.0 ? 1.0 - uv.y : uv.y;
	gl_Position = vec4(posuv.x * 0.45, posuv.y, depth, 1.0);
}