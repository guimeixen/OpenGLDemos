#include "Plane.h"

#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"

Plane::Plane(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3)
{
	Set3Points(v1, v2, v3);
}

Plane::Plane()
{
}

void Plane::Set3Points(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3)
{
	glm::vec3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = glm::cross(aux2, aux1);
	normal = glm::normalize(normal);

	pointP0 = v2;
	d = -(glm::dot(normal, pointP0));
}

void Plane::SetNormalAndPoint(const glm::vec3 &normal, const glm::vec3 &pointP0)
{
	this->normal = glm::normalize(normal);
	this->pointP0 = pointP0;
	d = -(glm::dot(this->normal, pointP0));
}

float Plane::Distance(glm::vec3 p)
{
	return (d + glm::dot(normal, p));
}

bool Plane::IntersectRay(const glm::vec3 &origin, const glm::vec3 &direction, float &t) const
{
	float denom = glm::dot(direction, normal);

	if (std::abs(denom) > 0.0001f)
	{
		glm::vec3 p0l0 = pointP0 - origin;
		t = glm::dot(p0l0, normal) / denom;
		return (t >= 0);
	}

	return false;
}
