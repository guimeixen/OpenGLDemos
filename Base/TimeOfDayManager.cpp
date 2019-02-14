#include "TimeOfDayManager.h"

#include "Input.h"

#include "glm\gtc\matrix_transform.hpp"
#include "GLFW\glfw3.h"

#include <iostream>

void TimeOfDayManager::Init()
{
	// Sunrise
	TimeInfo t7 = {};
	t7.time = 6.0f;
	t7.dirLightColor = glm::vec3(47.0f / 255.0f, 82.0f / 255.0f, 101.0f / 255.0f);
	t7.intensity = 0.04f;
	t7.ambient = 0.14f;
	t7.heightFogDensity = 0.0001f;
	t7.fogInscatteringColor = glm::vec3(8.0f / 255.0f, 16.0f / 255.0f, 61.0f / 255.0f);
	t7.lightInscaterringColor = glm::vec3(8.0f / 255.0f, 16.0f / 255.0f, 61.0f / 255.0f);
	t7.skyColor = glm::vec3(0.0f, 0.02f, 0.07f);
	t7.lightShaftsIntensity = 0.0f;

	TimeInfo t1 = {};
	t1.time = 7.1f;
	t1.dirLightColor = glm::vec3(255.0f / 255.0f, 80.0f / 255.0f, 0.0f);
	t1.intensity = 1.0f;
	t1.ambient = 0.14f;
	t1.heightFogDensity = 0.0001f;
	t1.fogInscatteringColor = glm::vec3(186.0f / 255.0f, 142.0f / 255.0f, 86.0f / 255.0f);
	t1.lightInscaterringColor = glm::vec3(253.0f / 255.0f, 110.0f / 255.0f, 0.0f);
	t1.skyColor = glm::vec3(0.09f, 0.015f, 0.0f);
	t1.lightShaftsIntensity = 0.31f;

	TimeInfo t2 = {};
	t2.time = 8.3f;
	t2.dirLightColor = glm::vec3(255.0f / 255.0f, 253.0f / 255.0f, 236.0f / 255.0f);
	t2.intensity = 0.8f;
	t2.ambient = 0.3f;
	t2.heightFogDensity = 0.0005f;
	t2.fogInscatteringColor = glm::vec3(138.0f / 255.0f, 195.0f / 255.0f, 250.0f / 255.0f);
	t2.lightInscaterringColor = glm::vec3(255.0f / 255.0f, 252.0f / 255.0f, 170.0f / 255.0f);
	t2.skyColor = glm::vec3(0.10f, 0.17f, 0.23f);
	t2.lightShaftsIntensity = 0.31f;

	// Sunset
	TimeInfo t8 = {};
	t8.time = 19.0f;
	t8.dirLightColor = glm::vec3(255.0f / 255.0f, 253.0f / 255.0f, 236.0f / 255.0f);
	t8.intensity = 0.8f;
	t8.ambient = 0.3f;
	t8.heightFogDensity = 0.0005f;
	t8.fogInscatteringColor = glm::vec3(138.0f / 255.0f, 195.0f / 255.0f, 250.0f / 255.0f);
	t8.lightInscaterringColor = glm::vec3(255.0f / 255.0f, 252.0f / 255.0f, 170.0f / 255.0f);
	t8.skyColor = glm::vec3(0.10f, 0.17f, 0.23f);
	t8.lightShaftsIntensity = 0.31f;

	TimeInfo t4 = {};
	t4.time = 20.0f;
	t4.dirLightColor = glm::vec3(255.0f / 255.0f, 80.0f / 255.0f, 0.0f);
	t4.intensity = 1.25f;
	t4.ambient = 0.14f;
	t4.heightFogDensity = 0.0001f;
	t4.fogInscatteringColor = glm::vec3(186.0f / 255.0f, 142.0f / 255.0f, 86.0f / 255.0f);
	t4.lightInscaterringColor = glm::vec3(253.0f / 255.0f, 110.0f / 255.0f, 0.0f);
	t4.skyColor = glm::vec3(0.09f, 0.015f, 0.0f);
	t4.lightShaftsIntensity = 0.31f;

	/*TimeInfo t9 = {};
	t9.time = 20.3f;
	t9.dirLightColor = glm::vec3(236.0f / 255.0f, 45.0f / 255.0f, 55.0f / 255.0f);
	t9.intensity = 1.25f;
	t9.ambient = 0.14f;
	t9.heightFogDensity = 0.0001f;
	t9.fogInscatteringColor = glm::vec3(186.0f / 255.0f, 142.0f / 255.0f, 86.0f / 255.0f);
	t9.lightInscaterringColor = glm::vec3(253.0f / 255.0f, 110.0f / 255.0f, 0.0f);
	t9.skyColor = glm::vec3(0.09f, 0.015f, 0.0f);
	t9.lightShaftsIntensity = 0.31f;*/

	TimeInfo t5 = {};
	t5.time = 21.1f;
	t5.dirLightColor = glm::vec3(47.0f / 255.0f, 82.0f / 255.0f, 101.0f / 255.0f);
	t5.intensity = 0.04f;
	t5.ambient = 0.14f;
	t5.heightFogDensity = 0.0001f;
	t5.fogInscatteringColor = glm::vec3(8.0f / 255.0f, 16.0f / 255.0f, 61.0f / 255.0f);
	t5.lightInscaterringColor = glm::vec3(8.0f / 255.0f, 16.0f / 255.0f, 61.0f / 255.0f);
	t5.skyColor = glm::vec3(0.0f, 0.02f, 0.07f);
	t5.lightShaftsIntensity = 0.0f;

	// Midnight
	TimeInfo t6 = {};
	t6.time = 24.0f;
	t6.dirLightColor = glm::vec3(47.0f / 255.0f, 82.0f / 255.0f, 101.0f / 255.0f);
	t6.intensity = 0.04f;
	t6.ambient = 0.14f;
	t6.heightFogDensity = 0.0001f;
	t6.fogInscatteringColor = glm::vec3(8.0f / 255.0f, 16.0f / 255.0f, 61.0f / 255.0f);
	t6.lightInscaterringColor = glm::vec3(8.0f / 255.0f, 16.0f / 255.0f, 61.0f / 255.0f);
	t6.skyColor = glm::vec3(0.0f, 0.02f, 0.07f);
	t6.lightShaftsIntensity = 0.0f;

	timeInfos[0] = t7;
	timeInfos[1] = t1;
	timeInfos[2] = t2;
	//timeInfos[3] = t3;
	timeInfos[3] = t8;
	timeInfos[4] = t4;
	//timeInfos[5] = t9;
	timeInfos[5] = t5;
	timeInfos[6] = t6;

	curIndex = 0;
	nextIndex = 1;
}

void TimeOfDayManager::Update(float dt)
{
	if (Input::IsKeyPressed(52))
		worldTime += dt * 2.8f;
	if (Input::IsKeyPressed(53))
		worldTime -= dt * 2.8f;

	/*if (Input::WasKeyPressed(GLFW_KEY_SPACE))
		incTime = !incTime;*/

	if (incTime)
		worldTime += dt * 0.05f;

	SetCurrentTime(worldTime);
	BlendTimeInfo(timeInfos[curIndex], timeInfos[nextIndex]);
}

void TimeOfDayManager::BlendTimeInfo(const TimeInfo &t1, const TimeInfo &t2)
{
	currentTimeInfo.dirLightColor = glm::mix(t1.dirLightColor, t2.dirLightColor, a);
	currentTimeInfo.ambient = glm::mix(t1.ambient, t2.ambient, a);
	//currentTime.dirLightDirection = glm::mix(t1.dirLightDirection, t2.dirLightDirection, a);
	currentTimeInfo.intensity = glm::mix(t1.intensity, t2.intensity, a);
	currentTimeInfo.heightFogDensity = glm::mix(t1.heightFogDensity, t2.heightFogDensity, a);
	currentTimeInfo.fogInscatteringColor = glm::mix(t1.fogInscatteringColor, t2.fogInscatteringColor, a);
	currentTimeInfo.lightInscaterringColor = glm::mix(t1.lightInscaterringColor, t2.lightInscaterringColor, a);
	currentTimeInfo.skyColor = glm::mix(t1.skyColor, t2.skyColor, a);
	currentTimeInfo.lightShaftsIntensity = glm::mix(t1.lightShaftsIntensity, t2.lightShaftsIntensity, a);
}

float TimeOfDayManager::DaysInMonth(float m, int y)
{
	m = m - 1;
	if (m == 0 || m == 2 || m == 4 || m == 6 || m == 7 || m == 9 || m == 11)
		return 31.0f;
	else if (m == 3 || m == 5 || m == 8 || m == 10)
		return 30.0f;
	else if (m == 1)
	{
		if (y % 4 == 0)
			return 29.0f;
		else if (y % 100 == 0)
			return 28.0f;
		else if (y % 400 == 0)
			return 29.0f;

		return 28.0f;
	}

	return 0.0f;
}

void TimeOfDayManager::DayOfYear()
{
	float sum = 0.0f;
	for (int i = 1; i < month; i++)
	{
		sum += DaysInMonth(i, year);
	}
	sum += day;
	float h = hour + offset + minute / 60.0f;
	dayOfYear = sum - 1 + (h - 12.0f) / 24.0f;
	//dayOfYear = sum;
}

void TimeOfDayManager::CalculateDayValues()
{
	float x = dayOfYear * 2 * 3.141592653f / 365.0f;			// fractional year in radians
	float eqTime = 229.18f * (0.000075f + 0.001868f * std::cos(x) - 0.032077f * std::sin(x) - 0.014615f * std::cos(2.0f * x) - 0.040849f * std::sin(2.0f * x));

	//std::cout << "Equation of time: " << eqTime << '\n';

	float declin = 0.006918f - 0.399912f * std::cos(x) + 0.070257f * std::sin(x) - 0.006758f * std::cos(2.0f * x);
	declin = declin + 0.000907f * std::sin(2.0f * x) - 0.002697f * std::cos(3.0f * x) + 0.00148f * std::sin(3.0f * x);
	declin = declin * 180 / 3.141592653f;

	//std::cout << "Declination angle: " << declin << '\n';

	// time offset in minutes
	float timeOffset = eqTime - 4.0f * -longi - 60.0f * offset;

	//std::cout << "Time offset in minutes: " << timeOffset << '\n';

	float solarTime = hour * 60.0f + minute + timeOffset;
	//solarTime /= 60.0f;

	//std::cout << "Solar time: " << solarTime / 60.0f << '\n';

	float hourAngle = solarTime / 4.0f - 180.0f;

	//std::cout << "Solar hour angle: " << hourAngle << '\n';

	// Zenith angle
	float k = 3.141592653f / 180.0f;
	float zenithAngle = std::sin(k * lat) * std::sin(k * declin) + std::cos(k * lat) * std::cos(k * declin) * std::cos(k * hourAngle);
	zenithAngle = std::acos(zenithAngle) / k;
	float altitudeAngle = 90 - zenithAngle;

	//std::cout << "Zenith angle: " << zenithAngle << '\n';
	//std::cout << "Altidude angle: " << altitudeAngle << '\n';

	// Azimuth angle
	float azimuthAngle = -(std::sin(k * lat) * std::cos(k * zenithAngle) - std::sin(k * declin)) / (std::cos(k * lat) * std::sin(k * zenithAngle));
	azimuthAngle = std::acos(azimuthAngle) / k;
	if (hourAngle < 0.0f)
		azimuthAngle = azimuthAngle;
	else
		azimuthAngle = 360.0f - azimuthAngle;

	azimuthAngle += azimuthOffset;

	//std::cout << "Azimuth angle: " << azimuthAngle << '\n';

	currentTimeInfo.dirLightDirection = glm::normalize(glm::vec3(std::cos(azimuthAngle*k) * std::cos(altitudeAngle*k), std::sin(altitudeAngle*k), -std::sin(azimuthAngle*k) * std::cos(altitudeAngle*k)));

	//std::cout << "Sun dir:  x: " << currentTimeInfo.dirLightDirection.x << "  y: " << currentTimeInfo.dirLightDirection.y << "  z: " << currentTimeInfo.dirLightDirection.z << '\n';
}

void TimeOfDayManager::SetCurrentTime(float time)
{
	worldTime = time;
	hour = std::floorf(worldTime);
	minute = (worldTime - (float)((int)worldTime)) * 60.0f;

	DayOfYear();
	CalculateDayValues();

	if (worldTime >= 24.0f || worldTime <= 0.0f)
	{
		worldTime = 0.0f;
	}
	float previousTime = 0.0f;
	float nextTime = 0.0f;

	for (size_t i = 0; i < MAX_TIME_INFOS; i++)
	{
		if (i == 0)
			previousTime = timeInfos[MAX_TIME_INFOS - 1].time;
		else
			previousTime = timeInfos[i - 1].time;

		nextTime = timeInfos[i].time;

		if (worldTime <= nextTime)
		{
			if (i == 0)
				curIndex = MAX_TIME_INFOS - 1;
			else
				curIndex = i - 1;

			nextIndex = i;

			if (previousTime == 24.0f)
				previousTime = 0.0f;

			float dif = nextTime - previousTime;		// Eg. nextTime = 18.0, previousTime = 12.0, dif = 6.0, worldTime = 15.0, dif2 = 18.0-15.0=3.0, a = 3/6 = 0.5, halfway through the blend
			float dif2 = nextTime - worldTime;
			a = dif2 / dif;
			a = 1.0f - a;
			break;
		}
	}
}

void TimeOfDayManager::SetAzimuthOffset(float offset)
{
	azimuthOffset = offset;
}
