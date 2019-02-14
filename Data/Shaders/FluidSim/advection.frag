#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 uv;

uniform float timeStep;

layout(binding = 0) uniform sampler2D velocityTexture;
layout(binding = 1) uniform sampler2D sourceTexture;

void main()
{
	vec2 coords = uv - timeStep * texture(velocityTexture, uv).rg;
	color = vec4(texture(sourceTexture, coords).rgb, 1.0);
}