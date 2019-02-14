#pragma once

#include "glm\glm.hpp"

class Plane
{
public:
	Plane();
	Plane(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3);

	void Set3Points(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3);
	void SetNormalAndPoint(const glm::vec3 &normal, const glm::vec3 &pointP0);
	float Distance(glm::vec3 p);

	bool IntersectRay(const glm::vec3 &origin, const glm::vec3 &direction, float &t) const;

	glm::vec3 GetNormal() const { return normal; }
	glm::vec3 GetPointP0() const { return pointP0; }

private:
	glm::vec3 normal;
	glm::vec3 pointP0;
	float d;
};
