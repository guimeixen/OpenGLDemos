#version 450 core

out vec4 outColor;

in vec2 uv;
in vec4 color;

layout(binding = 0) uniform sampler2D tex;

// Large text
//const float width = 0.51;
//const float edgeWidth = 0.02;

// Medium text
//const float width = 0.485;
//const float edgeWidth = 0.10;

// Small text
const float width = 0.46;
const float edgeWidth = 0.19;

void main()
{
	float dist = 1.0 - texture(tex, uv).a;	// The further a pixel is the higher the distance from the center. The alpha is the opposite it decreases the further from the center
														// For the correct distance we need to invert the alpha
															
	float alpha = 1.0 - smoothstep(width, width + edgeWidth, dist);	
	outColor = vec4(color.rgb, alpha * color.a);
}