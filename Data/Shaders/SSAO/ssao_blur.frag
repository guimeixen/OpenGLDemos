#version 450 core

layout(location = 0) out vec4 outValue;

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D ssaoTexture;

void main()
{
	vec2 texelSize = 1.0 / textureSize(ssaoTexture, 0);
	float result = 0.0;
	for (int x = -2; x < 2; x++)
	{
		for (int y = -2; y < 2; y++)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(ssaoTexture, uv + offset).r;
		}
	}
	outValue  =vec4(result / (4 * 4), 0.0, 0.0, 1.0);
}