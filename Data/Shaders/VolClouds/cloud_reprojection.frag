#version 450 core

layout(location = 0) out vec4 color;

layout (location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D quarterResCloudTexture;
layout(binding = 1) uniform sampler2D previousFrameCloudsTexture;

uniform mat4 invProj;
uniform mat4 invView;
uniform mat4 invProjView;
uniform mat4 proj;
uniform mat4 prevView;
uniform vec2 cloudRTDim;
uniform vec2 cloudSubRTDim;
uniform float updateBlockSize;
uniform float frameNumber;

void main()
{
	// We need to check if the current pixel was rendered to the low res buffer
	// If so we use that value for the pixel because it's the most up to date value
	// Otherwise we reproject the pixel using the result from the previous frame if it is inside the screen
	// and if it is outside we use the result from the up to date low res buffer
	
	vec2 scaledUV = floor(uv * cloudRTDim);
	vec2 uv2 = (floor(uv * cloudSubRTDim) + 0.5) / cloudSubRTDim;

	float x = mod(scaledUV.x, updateBlockSize);
	float y = mod(scaledUV.y, updateBlockSize);
	float frame = y * updateBlockSize + x;

	if(frame == frameNumber)
	{
		//color = vec4(0.0, 0.0,1.0, 1.0);
		color = texture(quarterResCloudTexture, uv2);
	}
	else
	{
		vec4 worldPos = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
		worldPos = invProj * worldPos;
		worldPos /= worldPos.w;
		worldPos = invView * worldPos;
	
		vec4 prevFramePos = proj * prevView * vec4(worldPos.xyz,1.0);
		prevFramePos.xy /= prevFramePos.w;
		prevFramePos.xy = prevFramePos.xy * 0.5 + 0.5;
	
		//bool isOut = any(greaterThan(abs(prevFramePos.xy - vec2(0.5)), vec2(0.5)));
		if(prevFramePos.x < 0.0 || prevFramePos.x > 1.0 || prevFramePos.y < 0.0 || prevFramePos.y > 1.0)
			//color = vec4(1.0,0.0,0.0,1.0);
			color = texture(quarterResCloudTexture, uv);
		else
			//color = vec4(0.0, 1.0, 0.0, 1.0);
			color = texture(previousFrameCloudsTexture, prevFramePos.xy);
	}
}