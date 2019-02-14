#pragma once

#include "glm\glm.hpp"

struct TimeInfo
{
	float time;
	glm::vec3 dirLightColor;
	float intensity;
	glm::vec3 dirLightDirection;
	float ambient;
	glm::vec3 fogInscatteringColor;
	float heightFogDensity;
	glm::vec3 lightInscaterringColor;
	glm::vec3 skyColor;
	float lightShaftsIntensity;
};

class TimeOfDayManager
{
public:
	void Init();
	void Update(float dt);

	void SetCurrentTime(float time);
	void SetAzimuthOffset(float offset);

	float GetCurrentTime() const { return worldTime; }
	float GetAzimuthOffset() const { return azimuthOffset; }

	const TimeInfo &GetCurrentTimeInfo() const { return currentTimeInfo; }


private:
	void BlendTimeInfo(const TimeInfo &t1, const TimeInfo &t2);

	float DaysInMonth(float m, int y);
	void DayOfYear();
	void CalculateDayValues();

private:
	static const unsigned int MAX_TIME_INFOS = 7;
	TimeInfo timeInfos[MAX_TIME_INFOS];
	TimeInfo currentTimeInfo;
	float a = 0.0f;
	unsigned int curIndex = 0;
	unsigned int nextIndex = 0;

	float worldTime = 13.0f;
	bool incTime = false;

	int year = 2018;
	float day = 13.0f;
	float month = 4.0f;
	float hour = 13.0f;
	float offset = 1.0f;
	float minute = 51.0f;
	float lat = 42.545f;
	float longi = -8.428f;
	float dayOfYear = 0.0f;
	float azimuth = 0.0f;
	float altitude = 0.0f;
	float azimuthOffset = 40.5f;
};
