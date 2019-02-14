#version 450 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

out vec2 uvGeom;
out vec3 normalGeom;
out vec3 worldPosGeom;
out vec4 lightSpacePosGeom;

uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

void main()
{
	normalGeom = inNormal;
	uvGeom = inUv;

	vec4 worldPos = modelMatrix * vec4(inPos, 1.0);
	worldPosGeom = worldPos.xyz;
	
	lightSpacePosGeom = lightSpaceMatrix * worldPos;;
	lightSpacePosGeom.xy = lightSpacePosGeom.xy * 0.5 + 0.5;				// z is already in [0,1] (glClipControl)
	
	gl_Position = worldPos;		// Does't matter what we put here, it will be changed in the geometry shader
}