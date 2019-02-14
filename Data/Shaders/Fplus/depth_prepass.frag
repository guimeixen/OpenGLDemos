#version 450 core

layout(location = 0) in float viewSpaceDepth;

void main()
{
	//gl_FragDepth = -viewSpaceDepth / 100.0;		// 100.0 = far plane
}