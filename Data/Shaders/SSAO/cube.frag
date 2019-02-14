#version 450 core

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in float viewSpaceDepth;
layout(location = 3) in vec3 worldPos;

//layout(binding = 0) uniform samplerCube skybox;
layout(binding = 0) uniform sampler2D diffuseTex;

uniform vec3 camPos;

void main()
{
	vec3 N = normalize(normal);
	outNormal = vec4(N, 1.0);
	
	vec3 V = normalize(camPos - worldPos);
	
	//outAlbedo.rgb = texture(skybox, reflect(N, -V)).rgb;
	outAlbedo.rgb = texture(diffuseTex, uv).rgb;
	//outAlbedo.rgb *= max(dot(N, normalize(vec3(1.0, 1.0, 1.0))),0.0) + 0.05;
	outAlbedo.a=1.0;
	
	gl_FragDepth = -viewSpaceDepth / 100.0;
	
}