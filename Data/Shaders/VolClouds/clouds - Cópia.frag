#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 camRay;

layout(binding = 0) uniform sampler3D noiseTexture;
layout(binding = 1) uniform sampler2D weatherTexture;
layout(binding = 2) uniform sampler3D highFreqNoiseTexture;
layout(binding = 3) uniform sampler2D highCloudsTexture;

uniform vec3 camPos;
uniform float cloudCoverage;
uniform float cloudStartHeight;
uniform float cloudLayerThickness;
uniform float cloudLayerTopHeight;
uniform vec3 lightDir;
uniform vec3 sunLightColor;
uniform float time;
uniform float timeScale;
uniform float hgForward;
uniform float hgBackward;
uniform float densityMult;
uniform float directLightMult;
uniform vec3 ambientTopColor;
uniform vec3 ambientBottomColor;
uniform float ambientMult;
uniform float detailScale;
//uniform float coneRadius;
uniform float highCloudsCoverage;
uniform float highCloudsTimeScale;
//uniform float lightningIntensity;
//uniform vec3 lightningPos;
//uniform vec3 randomInSphere[6];

const float planetSize = 350000.0;
const vec3 planetCenter =vec3(0.0, -planetSize, 0.0);
const float maxSteps = 128.0;
const float minSteps = 64.0;
const float baseScale = 0.00006;

vec3 raySphere(vec3 sc, float sr, vec3 ro, vec3 rd)
{
    vec3 oc = ro - sc;
    float b = dot(rd, oc);
    float c = dot(oc, oc) - sr*sr;
    float t = b*b - c;
    if( t > 0.0) 
        t = -b - sqrt(t);
    return ro + (c/t) * rd;
}

uint calcRaySphereIntersection(vec3 rayOrigin, vec3 rayDirection, vec3 sphereCenter, float radius, out vec2 t)
{
	vec3 l = rayOrigin - sphereCenter;
	float a = 1.0;
	float b = 2.0 * dot(rayDirection, l);
	float c = dot(l, l) - radius * radius;
	float discriminant = b * b - 4.0 * a * c;
	if (discriminant < 0.0)
	{
		t.x = t.y = 0.0;
		return 0;
	}
	else if (abs(discriminant) - 0.00005 <= 0.0)
	{
		t.x = t.y = -0.5 * b / a;
		return 1;
	}
	else
	{
		float q = b > 0.0 ? -0.5 * (b + sqrt(discriminant)) : -0.5 * (b - sqrt(discriminant));

		float h1 = q / a;
		float h2 = c / q;
		t.x = min(h1, h2);
		t.y = max(h1, h2);
		if (t.x < 0.0)
		{
			t.x = t.y;
			if (t.x < 0.0)
			{
				return 0;
			}
			return 1;
		}
		return 2;
	}
}

float remap(float original_value, float original_min, float original_max, float new_min, float new_max)
{
	return new_min + (((original_value - original_min) / (original_max - original_min)) * (new_max - new_min));
}

float beerTerm(float density)
{
	return exp(-density * densityMult);
}

float powderEffect(float density, float cosAngle)
{
	float powder = 1.0 - exp(-density * 2.0);
	return mix(1.0,  powder, clamp((-cosAngle * 0.5) + 0.5, 0.0, 1.0));
}

float HenyeyGreensteinPhase(float cosAngle, float g)
{
	float g2 = g * g;
	return ((1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosAngle, 1.5)) / 4.0 * 3.1415;
}

float SampleNoise(vec3 pos, float coverage, float cloudType, float height_0to1)
{
	pos.y *= baseScale;
	pos.x = pos.x * baseScale + time * timeScale;
	pos.z = pos.z * baseScale - time * timeScale;
	pos.y -= time * timeScale * 0.3;																								// Vertical motion
	//pos += height_0to1 * height_0to1 * height_0to1 * normalize(vec3(1.0, 0.0, 1.0)) * 0.3 * cloudType;				// Shear
	
	float height_1to0 = 1.0 - height_0to1;	
	
	vec4 noise = texture(noiseTexture, pos).rgba;
	
	//float verticalCoverage = texture(verticalCoverageTexture, vec2(cloudType, 1.0-height_0to1)).r;  // y is flipped
	
	float lowFreqFBM = noise.g * 0.625 + noise.b * 0.25 + noise.a * 0.125;
	float cloud = remap(noise.r, (1.0-lowFreqFBM), 1.0, 0.0, 1.0);
	cloud *= coverage;
	
	//float middleCoverage = height_0to1 * height_1to0 * 1.0;
	//cloud -= middleCoverage;			// Remove cloud at the middle to so they get a more anvil like shape
	//cloud += cloudType * height_0to1 * height_0to1 * 2.0;
	cloud *= cloudCoverage;
	
	vec3 detailCoord = pos * detailScale;	
	vec3 highFreqNoise = texture(highFreqNoiseTexture, detailCoord).rgb;
	float highFreqFBM = highFreqNoise.r * 0.625 + highFreqNoise.g * 0.25 + highFreqNoise.b * 0.125;

	cloud = remap(cloud, 1.0 - highFreqFBM * height_1to0, 1.0, 0.0, 1.0);							// Erode the edges of the clouds. Multiply by height_1to0 to erode more at the bottom and preserve the tops
	cloud +=  14.0 * cloudCoverage * coverage * cloudCoverage;
//	cloud *= verticalCoverage;
	cloud *= smoothstep(0.0, 0.2, height_0to1);				// Smooth the bottoms
	cloud = clamp(cloud, 0.0, 1.0);

	return cloud;
}

vec2 sampleWeather(vec3 pos)
{
	pos.x = pos.x * 0.000025 + time * timeScale;
	pos.z = pos.z * 0.000025 - time * timeScale;
	return texture(weatherTexture, pos.xz).rg;
}

float getNormalizedHeight(vec3 pos)
{
	return (distance(pos,  planetCenter) - (planetSize + cloudStartHeight)) / cloudLayerThickness;
}

bool sphereHit (vec3 p)
{
    return distance(p, vec3(0.0, 0.0, -2.0)) < 2.0;
}

vec4 clouds(vec3 dir)
{
	vec3 rayStart = vec3(0.0);
	vec3 rayEnd = vec3(0.0);
	vec3 rayPos = vec3(0.0);
	
	//rayStart  = raySphere(planetCenter, planetSize + cloudStartHeight, camPos , dir);
	//rayEnd = raySphere(planetCenter, planetSize + cloudLayerTopHeight, camPos , dir);
	//rayEnd = InternalRaySphereIntersect(planetSize + cloudLayerTopHeight, camPos , dir);

	float distanceCamPlanet = distance(camPos.xyz, planetCenter);
	
	vec2 ih = vec2(0.0);
	vec2 oh = vec2(0.0);
	
	uint innerShellHits = calcRaySphereIntersection(camPos.xyz, dir, planetCenter, planetSize + cloudStartHeight, ih);
	uint outerShellHits = calcRaySphereIntersection(camPos.xyz, dir, planetCenter, planetSize + cloudLayerTopHeight, oh);
	
	vec3 innerHit = camPos.xyz + dir * ih.x;
	vec3 outerHit = camPos.xyz + dir * oh.x;
	
	// Below the cloud layer
	if (distanceCamPlanet < planetSize + cloudStartHeight)
	{
		rayStart = innerHit;
		// Don't march if the ray is below the horizon
		if(rayStart.y < 0.0)
			return vec4(0.0);
			
		rayEnd = outerHit;
	}
	// Above the cloud layer
	/*else if (distanceCamPlanet > planetSize + cloudLayerTopHeight)
	{
		// We can enter the outer shell and leave through the inner shell or enter the outer shell and leave through the outer shell
		rayStart = outerHit;
		// Don't march iif we don't hit the outer shell
		if (outerShellHits == 0)
			return vec4(0.0);
		rayEnd = outerShellHits == 2 && innerShellHits == 0 ? camPos.xyz + dir * oh.y : innerHit;
	}
	// In the cloud layer
	else
	{
		rayStart = camPos.xyz;
		rayEnd = innerShellHits > 0 ? innerHit : outerHit;
	}*/
	
	float steps = int(mix(maxSteps, minSteps, dir.y));		// Take more steps when the ray is pointing more towards the horizon
	float stepSize = distance(rayStart, rayEnd) / steps;
	
	const float largeStepMult = 3.0;
	float stepMult = 1.0;
	
	rayPos = rayStart;
	
	const float cosAngle = dot(dir, lightDir);
	vec4 result = vec4(0.0);
	
	for (float i = 0.0; i < steps; i += stepMult)
	{
		if (result.a >= 0.99)
			break;
			
		float heightFraction = getNormalizedHeight(rayPos);
		
		/*if(heightFraction > 1.0)
			break;*/
		
		vec2 weatherData = sampleWeather(rayPos);
		float density = SampleNoise(rayPos, weatherData.r, weatherData.g, heightFraction);
		
		if (density > 0.0)
		{		
			float height_0to1_Light = 0.0;
			float densityAlongLight = 0.0;
			
			vec3 rayStep = lightDir * 40.0;
			vec3 pos  = rayPos + rayStep;
			
			float thickness = 0.0;
			float scale = 1.0;
			
			for (int s = 0; s < 5; s++)
			{
				pos += rayStep * scale;
				vec2 weatherData = sampleWeather(pos);
				height_0to1_Light = getNormalizedHeight(pos);
				densityAlongLight = SampleNoise(pos, weatherData.r, weatherData.g, height_0to1_Light);
				densityAlongLight *= float(height_0to1_Light <= 1.0);
				thickness += densityAlongLight;
				scale *= 4.0;
			}
			
			float direct = beerTerm(thickness) * powderEffect(density, cosAngle);
			float HG = mix(HenyeyGreensteinPhase(cosAngle, hgForward) * 0.3, HenyeyGreensteinPhase(cosAngle, hgBackward), 0.5);		// To make facing away from the sun more interesting
			direct *= HG * directLightMult;
		
			vec3 ambient = mix(ambientBottomColor, ambientTopColor, heightFraction) * ambientMult;
			
			vec4 lighting = vec4(density);
			lighting.rgb = direct * sunLightColor + ambient;
			//lighting.rgb = ambient;
			
			/*vec3 v = lightningPos - rayPos;
			float sqrDist = dot(v,v) ;
			float att = clamp(1.0 - sqrDist / 100000000.0, 0.0, 1.0);
			att *= att;
			lighting.rgb += att * lightningIntensity * (1.0-density);		// 1.0-density so areas with less clouds are brighter
			*/
			lighting.rgb *= lighting.a;
	
			result = lighting * (1.0 - result.a) + result;
		}
		
		rayPos += stepSize * dir * stepMult;
	}
	
	// Add high clouds
	uint highCloudHits = calcRaySphereIntersection(camPos.xyz, dir, planetCenter, planetSize + cloudLayerTopHeight + 2000.0, oh);
	rayPos = camPos.xyz + dir * oh.x;
	rayPos.x = rayPos.x * 0.00005 /*+ time * highCloudsTimeScale;*/;
	rayPos.z = rayPos.z * 0.00005 + time * highCloudsTimeScale;
	float highClouds = texture(highCloudsTexture, rayPos.xz).r;
	highClouds *= highClouds * highClouds;
	highClouds *= highCloudsCoverage;
	
	//highClouds *= float(rayPos.y > 0.0);
	
	float HG = mix(HenyeyGreensteinPhase(cosAngle, hgForward) * 0.3 , HenyeyGreensteinPhase(cosAngle, hgBackward), 0.5) ;
	vec3 col = highClouds * HG * sunLightColor * directLightMult;
	
	result.rgb = col * (1.0 - result.a) + result.rgb;
	
	return result;
}

float SampleRain(vec3 pos, float coverage, float cloudType)
{
	return coverage * 0.5 * coverage * cloudCoverage * cloudCoverage;
}

vec4 rain(vec3 dir)
{
	vec3 rayPos = vec3(0.0);
	vec3 rayEnd = vec3(0.0);
	
	float distanceCamPlanet = distance(camPos.xyz, planetCenter);

	//rayPos = calcRaySphereIntersection(camPos.xyz, dir, planetCenter, planetSize);
	
	if(rayPos.y < 0.0)
		return vec4(0.0);
			
	//rayEnd = calcRaySphereIntersection(camPos.xyz, dir, planetCenter, planetSize + cloudStartHeight);
	
	//steps = int(mix(maxSteps, minSteps, dir.y));		// Take more steps when the ray is pointing more towards the horizon
	const int steps = 8;
	const float stepSize = distance(rayPos, rayEnd) / steps;
	
	//const vec3 lightPos = vec3(0.0, 2000.0, 0.0);
	//const float cosAngle = dot(dir, lightDir);
	vec4 res = vec4(0.0);
	
	for (int i = 0; i < steps; i++)
	{
		if (res.a >= 0.99)
			break;
			
		if(rayPos.y < 0.0)
			break;
			
		vec2 weatherData = sampleWeather(rayPos);		
		float density = SampleRain(rayPos, weatherData.r, weatherData.g);
	//	if(density>0.0)
			res = (1.0 - res.a) * vec4(density, density, density, clamp(density*5.0,0.0,1.0))  + res;
		
		rayPos += stepSize * dir;
	}
	res.rgb *=  ambientBottomColor;
	return res;
}

void main()
{
	vec3 dir = normalize(camRay);
	color =  clouds(dir);
	/*color=vec4(0.0,0.0,0.0,1.0);
	vec3 ro = camPos;
	float stepSize = 0.1;
	for (int i = 0; i < 16; i++)
	{
		if(sphereHit(ro))
		{
			color.r = 1.0;
			return;
		}
		ro += stepSize * dir;
	}*/
	
	//vec4 rain = rain(dir);

	//color.rgb = color.rgb * (1.0 - rain.a) + rain.rgb * sunLightColor;
	//color.rgb = mix(color.rgb, sunLightColor, clamp(-dir.y * 20.0 + 0.9, 0.0, 1.0));
}