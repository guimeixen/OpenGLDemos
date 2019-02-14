#version 450 core

layout(location = 0) in vec4 posuv;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 camRay;

uniform mat4 invProj;
uniform mat4 invView;

vec3 UVToCamRay()
{
	vec4 clipSpacePos = vec4(posuv.zw * 2.0 - 1.0, 1.0, 1.0);
	vec4 viewSpacePos = invProj * clipSpacePos;
	viewSpacePos.xyz /= viewSpacePos.w;
	vec4 worldSpacePos = invView * viewSpacePos;
	
	return worldSpacePos.xyz;
}

void main()
{
	uv = posuv.zw;
	camRay = UVToCamRay();
	gl_Position = vec4(posuv.xy, 0.0, 1.0);
}