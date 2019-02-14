#version 450 core

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 clipSpacePos;
layout(location = 3) in vec3 worldPos;

layout(binding = 0) uniform sampler2D reflectionTexture;
layout(binding = 1) uniform sampler2D refractionTexture;
layout(binding = 2) uniform sampler2D refractionDepthTexture;
layout(binding = 3) uniform sampler2D normalMap;

uniform vec3 camPos;
uniform float time;
uniform vec4 normalMapOffset;
uniform vec3 sunDir;
uniform vec3 sunLightColor;

const float twoPI = 2 * 3.14159;
const float wavelength[4] = float[](13.0, 9.9, 7.3, 6.0);
const float amplitude[4] = float[](0.41, 0.32, 0.17, 0.09);
const float speed[4] = float[](3.4, 2.8, 1.8, 0.6);
const vec2 direction[4] = vec2[](vec2(1.0, -0.2), vec2(1.0, 0.6), vec2(-0.2, 1.0), vec2(-0.43, -0.8));

const float near = 0.2;
const float far = 1000.0;

float LinearizeDepth(float depth)
{
	return 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));
}

vec3 waveNormal(vec2 pos)
{
	float x = 0.0;
	float z = 0.0;
	float y = 0.0;
		
    for (int i = 0; i < 4; ++i)
	{
		float frequency = twoPI / wavelength[i];
		float phase = speed[i] * frequency;
		float q = 0.98 / (frequency * amplitude[i] * 4);
		
		float DdotPos = dot(direction[i], pos);
		float c = cos(frequency * DdotPos + phase * time);
		float s = sin(frequency * DdotPos + phase * time);
	
		float term2 = frequency * amplitude[i] * c;
		x += direction[i].x * term2;
		z += direction[i].y * term2;
		y += q * frequency * amplitude[i] * s;
	}

    return normalize(vec3(-x, 1 - y, -z));
}

void main()
{
	vec3 L = sunDir;
	vec3 V = normalize(camPos - worldPos);
	vec3 H = normalize(L + V);

	vec3 N = waveNormal(worldPos.xz);
	/*float detailFalloff = clamp((clipSpacePos.w - 60.0) / 40.0, 0.0, 1.0);
    N = normalize(mix(N, vec3(0.0, 1.0, 0.0), detailFalloff));

	vec2 vTex = worldPos.xz * 0.02;
	vec4 wave0 = vTex.xyxy * vec4(4, 4, 2, 2) + normalMapOffset.xyzw * vec4(1, 1, 2, 2);
	vec4 wave1 = wave0 * 2;
	
	vec3 bumpNormal;
	vec2 bumpColorA = texture(normalMap, wave0.xy).rg * 2.0 - 1.0;
	vec2 bumpColorB = texture(normalMap, wave0.zw).rg * 2.0 - 1.0;

	vec2 bumpLowFreq = (bumpColorA + bumpColorB) ;
                
	vec2 bumpColorC = texture(normalMap, wave1.xy).rg * 2.0 - 1.0;             
	vec2 bumpColorD = texture(normalMap, wave1.zw).rg * 2.0 - 1.0;           
  
	vec2 bumpHighFreq = (bumpColorC + bumpColorD) ;

 	bumpNormal = vec3(bumpLowFreq.x + bumpHighFreq.x, 1, bumpLowFreq.y + bumpHighFreq.y);

	bumpNormal.xz *= 0.25;

	vec3 tangent = cross(N, vec3(0.0, 0.0, 1.0));
    vec3 bitangent = cross(tangent, N);
    N = tangent * bumpNormal.x + N * bumpNormal.y + bitangent * bumpNormal.z;
	N = normalize(N);*/
	
	/*vec3 fineNormal = texture(normalMap, worldPos.xz*0.05+time*0.05).rgb;
	fineNormal = fineNormal * 2.0 - 1.0;		// From RGB [0,1] to [-1,1]
	//mat3 tbn = mat3(tangent, bitangent, normal);
	//vec3 N = normal + fineNormal * tbn;
	vec3 N = normal + fineNormal.x * tangent + fineNormal.y * bitangent;
	N = normalize(N);*/
	
	vec2 tex1 = (worldPos.xz * 0.035 + normalMapOffset.xy);
	vec2 tex2 = (worldPos.xz * 0.01 + normalMapOffset.zw);

	vec3 normal1 = texture(normalMap, tex1).rbg * 2.0 - 1.0;
	vec3 normal2 = texture(normalMap, tex2).rbg * 2.0 - 1.0;
	vec3 fineNormal = normal1 + normal2;

	float detailFalloff = clamp((clipSpacePos.w - 60.0) / 40.0, 0.0, 1.0);
    fineNormal = normalize(mix(fineNormal, vec3(0.0, 2.0, 0.0), clamp(detailFalloff - 8.0, 0.0, 1.0)));
    N = normalize(mix(N, vec3(0.0, 1.0, 0.0), detailFalloff));

	
	vec3 shallowColor = vec3(0.38, 1.0, 0.86);
	vec3 deepColor = vec3(0.11, 0.28, 0.54);
	
	vec3 sss = clamp(dot(V,-L),0.0,1.0) * (1.0-N.y) * shallowColor;
	
	
	
	// Transform fine normal to world space
    vec3 tangent = cross(N, vec3(0.0, 0.0, 1.0));
    vec3 bitangent = cross(tangent, N);
    N = tangent * fineNormal.x + N * fineNormal.y + bitangent * fineNormal.z;
	N = normalize(N);
	
	vec2 offset = N.xz * vec2(0.05, -0.05);
	
	vec2 ndc = clipSpacePos.xy / clipSpacePos.w * 0.5 + 0.5;
	vec2 reflectionUV = vec2(ndc.x, 1.0 - ndc.y);
	vec2 refractionUV = ndc;
	
	reflectionUV += offset;
	refractionUV += offset;
	
	reflectionUV = clamp(reflectionUV, 0.001, 0.999);
	refractionUV = clamp(refractionUV, 0.001, 0.999);
	
	vec3 reflection = texture(reflectionTexture, reflectionUV).rgb;
	vec3 refraction = texture(refractionTexture, refractionUV).rgb;
	float depth = texture(refractionDepthTexture, refractionUV).r;
	
	float waterBottomDistance = LinearizeDepth(depth);
	
	depth = gl_FragCoord.z;
	float waterTopDistance = LinearizeDepth(depth);

	float waterDepth = waterBottomDistance - waterTopDistance;
	//waterDepth *= 0.2;
	
	
	float range = clamp(0.3 * waterDepth, 0.0, 1.0);
	range = 1.0 - range;
	range = mix(range, range * range * range, 0.5);
	
	vec3 col = mix(deepColor, shallowColor, range);
	//col = pow(col, vec3(2.2));
	//refraction = pow(refraction, vec3(1.0/2.2));
	refraction *= col;

	float diffuse = max(dot(N, L),0.0);
	
	float spec = pow(max(dot(N, H), 0.0), 256.0);
	vec3 specular = vec3(spec) * sunLightColor;
	
	// Fresnel Effect
	float NdotV = max(dot(N, V), 0.0);
	float fresnel = pow(1.0 - NdotV, 5.0) * 0.5;
	fresnel = clamp(fresnel, 0.0, 1.0);
	
	
	
	vec3 color = mix(refraction, reflection, fresnel);
	//vec3 color = vec3(diffuse);
	//color.rgb *= diffuse;
	color.rgb += specular;
	color.rgb += sss;
	//color.rgb*=sunLightColor;
	
	outColor = vec4(color, 1.0);
}