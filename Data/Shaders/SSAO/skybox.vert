#version 450 core

layout (location = 0) in vec3 position;

layout(location = 0) out vec3 uv;

uniform mat4 projView;

void main()
{
	vec4 pos = projView * vec4(position, 1.0);
	gl_Position = pos.xyww;
	uv = position;
}