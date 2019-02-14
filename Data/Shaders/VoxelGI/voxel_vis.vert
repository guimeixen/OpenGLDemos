#version 450 core

layout (location = 0) in vec3 inPos;

out vec3 worldPos;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

void main()
{
	vec4 worldPos = modelMatrix * vec4(inPos, 1.0);
	worldPos = worldPos.xyz;
	gl_Position = projectionMatrix * viewMatrix * worldPos;
}