#version 450

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec3 worldPos;

uniform mat4 lightTransform;
uniform mat4 modelMatrix;

void main()
{
	vec4 wPos = modelMatrix * vec4(inPos, 1.0);
	worldPos = wPos.xyz;
	gl_Position = lightTransform * wPos;
}