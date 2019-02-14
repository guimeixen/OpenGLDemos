#version 450

out vec4 outColor;

layout(location = 0) in vec2 uv;

layout(binding = 4, rg32ui) uniform readonly uimage2D oLightGrid;
//layout(binding = 6, r16f) uniform readonly image2D debugTex;

void main()
{
	ivec2 size = imageSize(oLightGrid);

	uvec2 offsetAndLightCount = imageLoad(oLightGrid, ivec2(uv.x * size.x, uv.y * size.y)).xy;
	
	if (offsetAndLightCount.y > 50)
	{
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else if (offsetAndLightCount.y > 25)
	{
		outColor = vec4(1.0, 0.5, 0.0, 1.0);
	}
	else if (offsetAndLightCount.y > 15)
	{
		outColor = vec4(1.0, 1.0, 0.0, 1.0);
	}
	else if (offsetAndLightCount.y > 5)
	{
		outColor = vec4(0.0, 1.0, 0.0, 1.0);
	}
	else if (offsetAndLightCount.y > 0)
	{
		outColor = vec4(0.0, 0.7, 1.0, 1.0);
	}
	else
	{
		outColor = vec4(0.0, 0.0, 1.0, 1.0);
	}
	
	//outColor.rgb = imageLoad(debugTex, ivec2(uv.x * 40, uv.y * 30)).rrr;
	//outColor.a=1.0;
}