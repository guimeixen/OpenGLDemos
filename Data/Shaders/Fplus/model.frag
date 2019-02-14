#version 450

out vec4 outColor;

struct Light
{
	//uint type;
	vec4 positionWS;
	vec4 positionVS;
	vec4 color;
	//float radius;
};

layout(binding = 1, std430) readonly buffer OpaqueLightIndexList
{
	uint oLightIndexList[];
};

layout(binding = 3, std140) readonly buffer LightsSSBO
{
	Light lights[];
};

layout(binding = 4, rg32ui) uniform readonly uimage2D oLightGrid;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 worldPos;

uniform sampler2D tex;

//uniform vec4 lightP;
//uniform uint numLights;

void main()
{
	vec3 N = normalize(normal);
	
	vec4 diffuseTex = texture(tex, uv);
	
	if (diffuseTex.a < 0.5)
		discard;
	
	// Get the index of the current pixel in the light grid
	ivec2 tileIndex = ivec2(floor(gl_FragCoord.xy / 16));
	
	// Get the start offset and the light count for this pixel in the light index list
	uvec2 offsetAndLightCount = imageLoad(oLightGrid, tileIndex).xy;
	
	vec3 lighting = vec3(0.0);
	
	for (uint i = 0; i < offsetAndLightCount.y; i++)
	{
		uint lightIndex = oLightIndexList[offsetAndLightCount.x + i];
		Light light = lights[lightIndex];
		
		vec3 toLight = light.positionWS.xyz - worldPos;
		vec3 L = normalize(toLight);
		vec3 diff = vec3(max(dot(N,L), 0.0)) * 6.0 * light.color.xyz;
		float dist = length(toLight);
		float att = clamp(1.0 - dist / light.positionWS.w, 0.0, 1.0);
		att *= att;
		diff *= att;
		
		lighting += diff;
	}
	
	lighting += vec3(0.35);
	lighting *= diffuseTex.rgb;
	
	outColor = vec4(lighting, 1.0);
	//outColor = texture(tex, uv);
	//outColor = vec4(normal, 1.0);
}