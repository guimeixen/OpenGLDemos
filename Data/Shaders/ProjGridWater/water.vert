#version 450 core

layout (location = 0) in vec2 inUv;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec4 clipSpacePos;
layout(location = 3) out vec3 worldPos;

uniform mat4 projectionMatrix;
uniform mat4 projView;
uniform mat4 viewFrame;
uniform vec4 viewCorner0;
uniform vec4 viewCorner1;
uniform vec4 viewCorner2;
uniform vec4 viewCorner3;
uniform float time;
uniform vec3 camPos;

const float twoPI = 2 * 3.14159;
const float wavelength[4] = float[](13.0, 9.9, 7.3, 6.0);
const float amplitude[4] = float[](0.41, 0.32, 0.17, 0.09);
const float speed[4] = float[](3.4, 2.8, 1.8, 0.6);
const vec2 direction[4] = vec2[](vec2(1.0, -0.2), vec2(1.0, 0.6), vec2(-0.2, 1.0), vec2(-0.43, -0.8));

vec3 wave(vec2 pos)
{
	vec3 wave = vec3(0.0);

    for (int i = 0; i < 4; ++i)
	{
		float frequency = twoPI / wavelength[i];
		float phase = speed[i] * frequency;
		float q = 0.98 / (frequency * amplitude[i] * 4);
		
		float DdotPos = dot(direction[i], pos);
		float c = cos(frequency * DdotPos + phase * time);
		float s = sin(frequency * DdotPos + phase * time);
		
		float term = q * amplitude[i] * c;
		
		wave.x += term * direction[i].x;
		wave.z += term * direction[i].y;
		wave.y += amplitude[i] * s;
	}

    return wave;
}

void main()
{
	uv = inUv;

	vec4 pos = mix(mix(viewCorner1, viewCorner0, inUv.x), mix(viewCorner2, viewCorner3, inUv.x), inUv.y);
	pos.xyz /= pos.w;
	worldPos = pos.xyz;
	
	float l = length(camPos - worldPos);
	worldPos += wave(worldPos.xz) *  (1.0 - smoothstep(55.0, 100.0, l));
	
	clipSpacePos = projView * vec4(worldPos, 1.0);
	gl_Position = clipSpacePos;
}