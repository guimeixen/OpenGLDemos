#version 450 core

out vec4 color;

in vec2 uv;
in vec3 normal;
in vec3 worldPos;
in vec3 lightSpacePos;

layout(binding = 0) uniform sampler2D texDiffuse;
layout(binding = 1) uniform sampler2D texSpec;
layout(binding = 3) uniform sampler3D voxelTexture;
layout(binding = 4) uniform sampler2DShadow depthTexture;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 camPos;
uniform float volumeSize;
uniform float samplingFactor = 1.0;
uniform float bounceStrength = 1.0;
uniform float aoFalloff = 725.0;
uniform bool enableIndirect = true;
uniform float specAperture = 0.4;
uniform float diffAperture = 0.67;

const float voxelScale = 1.0 / 32.0;

const vec3 diffuseConeDirections[] =
{
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.5, 0.86602),
    vec3(0.823639, 0.5, 0.267617),
    vec3(0.509037, 0.5, -0.7006629),
    vec3(-0.50937, 0.5, -0.7006629),
    vec3(-0.823639, 0.5, 0.267617)
};

const float diffuseConeWeights[] =
{
    3.14159 / 4.0,
    3.0 * 3.14159 / 20.0,
    3.0 * 3.14159 / 20.0,
    3.0 * 3.14159 / 20.0,
    3.0 * 3.14159 / 20.0,
    3.0 * 3.14159 / 20.0,
};

vec3 orthogonal(vec3 u)
{
	//u = normalize(u);
	vec3 v = vec3(0.99146, 0.11664, 0.05832);
	return abs(dot(u, v)) > 0.999 ? cross (u, vec3(0.0, 1.0, 0.0)) : cross(u, v);
}

vec3 worldToVoxel(vec3 position)
{
	vec3 voxelGridMinPoint = vec3(-16.0);
	vec3 voxelPos = position - voxelGridMinPoint;
	return voxelPos * voxelScale;
}

float shadowCalc()
{
	vec3 uv = vec3(lightSpacePos.xy, lightSpacePos.z - 0.005);
	float shadow = texture(depthTexture, uv).r;
	
	/*vec2 pixelSize = 1.0 / textureSize(depthTexture, 0);
	float shadow = 0.0;
	vec3 tempUv;
    for(int x = -1; x <= 1; x++)
	{
        for(int y = -1; y <= 1; y++)
		{
			tempUv = uv;
			tempUv.xy += vec2(x,y) * pixelSize;
            shadow += texture(depthTexture, tempUv).r;
        }
    }
	shadow /= 9.0;*/
	
	
	//float closestDepth = texture(depthTexture, lightSpacePos.xy).r;
	//float currentDepth = lightSpacePos.z;
	
	//float bias = 0.005;
	//float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	
	return shadow;
}

vec4 traceCone(vec3 position, vec3 N, vec3 dir, float aperture)
{
	float voxelWorldSize = 2.0 / (voxelScale * volumeSize);		// 0.5
	
	// Move a bit to avoid self collision
	float dist = voxelWorldSize * 2.0;				// 1.0
	vec3 startPos = position + N * dist;
	
	vec4 coneSample = vec4(0.0);
	float occlusion = 0.0;
	float maxDistance = 1.0 / voxelScale;
	float falloff = 0.5 * aoFalloff * voxelScale;
	
	while(coneSample.a < 1.0 && dist <= maxDistance)
	{
		vec3 conePosition = startPos + dir * dist;
		
		// Cone expansion and respective mipmap level based on diameter
		float diameter = 2.0 * aperture * dist;
		float mipLevel = log2(diameter / voxelWorldSize);
		
		// convert position to texture coord
		vec3 uvw = worldToVoxel(conePosition);
		
		vec4 value = textureLod(voxelTexture, uvw, mipLevel);
		
		// front to back composition
		coneSample += (1.0 - coneSample.a) * value;
		occlusion += ((1.0 - occlusion) * value.a) / (1.0 + falloff * diameter);
		
		// move further into the volume
		dist += diameter * samplingFactor;
	}
	
	return vec4(coneSample.rgb, occlusion);
}

vec4 indirectDiffuseLight(vec3 N)
{
	// Find a base for the side cones with the normal as one of it's base vectors
	const vec3 tangent = normalize(orthogonal(N));
	const vec3 bitangent = cross(tangent, N);
	
	vec4 indDiffuse = vec4(0.0);
	vec3 coneDir;
	
	for(int i = 0; i < 6; i++)
    {
         coneDir = N;
         coneDir += diffuseConeDirections[i].x * tangent + diffuseConeDirections[i].z * bitangent;
         coneDir = normalize(coneDir);
         indDiffuse += traceCone(worldPos, N, coneDir, diffAperture) * diffuseConeWeights[i];
    }
	
	indDiffuse.a = clamp(1.0 - indDiffuse.a + 0.01, 0.0, 1.0);
	indDiffuse.rgb *= bounceStrength;
	return indDiffuse;
}

vec4 indirectSpecularLight(vec3 V, vec3 N)
{
	vec3 coneDir = reflect(-V, N);
	coneDir = normalize(coneDir);
	
	float spec = texture(texSpec, uv).r;
	
	spec = clamp(spec, 0.0, 0.98);
	
	float aperture = clamp(tan(1.57079 * (1.0 - spec)), 0.0174533, 3.14159);
	vec4 specular = traceCone(worldPos, N, coneDir, aperture);
	
	return specular * bounceStrength;
}

void main()
{
	vec4 diffuseTex = texture(texDiffuse, uv);
	if (diffuseTex.a < 0.5)
		discard;

	color = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 N = normalize(normal);
	vec3 V = normalize(camPos - worldPos);
	
	vec4 indirectDiffuse = indirectDiffuseLight(N);
	vec4 indirectSpecular = indirectSpecularLight(V, N);
	
	vec3 directDiffuse = max(dot(N, lightDir), 0.0) * lightColor;
	
	color.rgb = directDiffuse * shadowCalc();
	if (enableIndirect)
	{
		color.rgb += indirectDiffuse.rgb + indirectSpecular.rgb;
		color.rgb *= vec3(indirectDiffuse.a);
	}
	color.rgb *= diffuseTex.rgb;
	//color.rgb = vec3(indirectDiffuse.a);
	
	//color.rgb = vec3(texture(texSpec, uv).r);
	
	color.rgb = pow(color.rgb, vec3(0.454545));
	color.a = 1.0;
}