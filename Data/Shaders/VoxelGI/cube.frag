#version 450 core

layout(location = 0) out vec4 outColor;

in vec3 color;

void main()
{
	outColor = vec4(color, 1.0);
	outColor.rgb = pow(outColor.rgb, vec3(0.45));
}