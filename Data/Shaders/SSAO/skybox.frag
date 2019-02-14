#version 450 core

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

layout(location = 0) in vec3 uv;

layout(binding = 0) uniform samplerCube skybox;

void main()
{
    outColor = texture(skybox, uv);
	outNormal=vec4(0.0,0.0,0.0,1.0);
}