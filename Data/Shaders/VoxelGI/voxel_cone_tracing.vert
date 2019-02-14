#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 inNormal;

out vec2 uv;
out vec3 normal;
out vec3 worldPos;
out vec3 lightSpacePos;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

void main()
{
	normal = inNormal;
	uv = texCoord;

	vec4 worldPos4 = modelMatrix * vec4(position, 1.0);
	worldPos = worldPos4.xyz;
	
	vec4 lightSpacePos4 = lightSpaceMatrix * worldPos4;
	lightSpacePos = lightSpacePos4.xyz;
	lightSpacePos.xy = lightSpacePos.xy * 0.5 + 0.5;		// z is already in [0,1] (glClipControl)
	
	gl_Position = projectionMatrix * viewMatrix * worldPos4;
}