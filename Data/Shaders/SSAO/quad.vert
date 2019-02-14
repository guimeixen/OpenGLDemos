#version 450 core

layout(location = 0) in vec4 posuv;
layout(location = 1) in int cornerID;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 viewRay;

uniform vec3 frustumFarCornersViewSpace[4];

void main()
{
	uv = posuv.zw;
	viewRay = frustumFarCornersViewSpace[cornerID];
	gl_Position = vec4(posuv.xy, 0.0, 1.0);
}