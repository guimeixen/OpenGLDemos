#version 450

layout(location = 0) in vec3 worldPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
	float lightDist = length(lightPos - worldPos);		// Distance to the light
	// map to [0,1] range
	lightDist = lightDist / farPlane;		// far plane
	gl_FragDepth = lightDist;
}