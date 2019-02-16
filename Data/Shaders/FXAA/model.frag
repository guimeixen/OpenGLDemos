#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D diffuse;

void main()
{
	color.rgb = pow(texture(diffuse, uv).rgb, vec3(0.45));
	color.a = 1.0;
}