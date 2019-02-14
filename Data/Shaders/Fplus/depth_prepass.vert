#version 450 core

layout(location = 0) in vec3 pos;

layout(location = 0) out float viewSpaceDepth;

uniform mat4 modelMatrix;
uniform mat4 proj;
uniform mat4 view;

void main()
{
	vec4 posViewSpace = view * modelMatrix * vec4(pos, 1.0);
	viewSpaceDepth = posViewSpace.z;
	gl_Position = proj * posViewSpace;
}