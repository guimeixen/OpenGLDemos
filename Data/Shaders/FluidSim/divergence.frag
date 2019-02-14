#version 450

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 uv;

layout(binding = 0) uniform sampler2D velocityTexture;

void main()
{
	ivec2 coord = ivec2(gl_FragCoord).xy;
	
	vec2 n = texelFetchOffset(velocityTexture, coord, 0, ivec2(0, 1)).rg;
	vec2 s = texelFetchOffset(velocityTexture, coord, 0, ivec2(0, -1)).rg;
	vec2 e = texelFetchOffset(velocityTexture, coord, 0, ivec2(1, 0)).rg;
	vec2 w = texelFetchOffset(velocityTexture, coord, 0, ivec2(-1, 0)).rg;
	
	color = vec4(vec3(e.x - w.x + n.y - s.y), 1.0);
}