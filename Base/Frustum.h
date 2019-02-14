#pragma once

#include "Plane.h"
//#include "AABB.h"

class Frustum
{
public:
	enum { OUTSIDE, INTERSECT, INSIDE };

	Frustum();
	~Frustum();

	void SetCamInternals(float fov, float aspectRatio, float nearD, float farD);
	void SetCamDef(glm::vec3 p, glm::vec3 l, glm::vec3 u);
	int SphereInFrustum(glm::vec3 sphereCenter, float radius);
	//int BoxInFrustum(const AABB &box);

	glm::vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;

private:
	enum { TOP, BOTTOM, LEFT, RIGHT, NEARP, FARP };

	Plane pl[6];
	float nearD, farD, aspectRatio, fov, tang;
	float nw, nh, fw, fh;
};
