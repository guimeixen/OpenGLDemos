#version 450 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 worldPos;
layout(location = 3) out mat3 TBN;

uniform mat4 projView;
uniform mat4 modelMatrix;

void main()
{
	uv = inUv;
	normal = inNormal;
	
	vec3 T = normalize(vec3(modelMatrix * vec4(inTangent,   0.0)));
	//vec3 B = normalize(vec3(modelMatrix * vec4(inBitangent, 0.0)));
	vec3 N = normalize(vec3(modelMatrix * vec4(inNormal,    0.0)));
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	// then retrieve perpendicular vector B with the cross product of T and N
	vec3 B = cross(N, T);
	
	TBN = mat3(T,B,N);
	
	vec4 wPos = modelMatrix * vec4(inPos, 1.0);
	worldPos = wPos.xyz;
	gl_Position = projView * wPos;
}

