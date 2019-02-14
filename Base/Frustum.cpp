#include "Frustum.h"

#include <glm\gtc\matrix_access.hpp>
#include <glm\glm.hpp>


#include <iostream>

Frustum::Frustum()
{
}

Frustum::~Frustum()
{
}

void Frustum::SetCamInternals(float fov, float aspectRatio, float nearD, float farD)
{
	this->aspectRatio = aspectRatio;
	this->fov = fov;
	this->nearD = nearD;
	this->farD = farD;

	tang = glm::tan(glm::radians(fov * 0.5f));
	nh = nearD * tang;
	nw = nh * aspectRatio;
	fh = farD * tang;
	fw = fh * aspectRatio;
}

void Frustum::SetCamDef(glm::vec3 p, glm::vec3 l, glm::vec3 u)
{
	glm::vec3 nc, fc, X, Y, Z;

	Z = p - l;
	Z = glm::normalize(Z);

	X = glm::cross(u, Z);
	X = glm::normalize(X);

	Y = glm::cross(Z, X);

	nc = p - Z * nearD;
	fc = p - Z * farD;

	ntl = nc + Y * nh - X * nw;
	ntr = nc + Y * nh + X * nw;
	nbl = nc - Y * nh - X * nw;
	nbr = nc - Y * nh + X * nw;

	ftl = fc + Y * fh - X * fw;
	ftr = fc + Y * fh + X * fw;
	fbl = fc - Y * fh - X * fw;
	fbr = fc - Y * fh + X * fw;

	pl[TOP].Set3Points(ntr, ntl, ftl);
	pl[BOTTOM].Set3Points(nbl, nbr, fbr);
	pl[LEFT].Set3Points(ntl, nbl, fbl);
	pl[RIGHT].Set3Points(nbr, ntr, fbr);
	pl[NEARP].Set3Points(ntl, ntr, nbr);
	pl[FARP].Set3Points(ftr, ftl, fbl);
}

int Frustum::SphereInFrustum(glm::vec3 sphereCenter, float radius)
{
	int result = INSIDE;
	float distance;

	for (int i = 0; i < 6; i++)
	{
		distance = pl[i].Distance(sphereCenter);
		if (distance < -radius)
			return OUTSIDE;
		else if (distance < radius)
			result = INTERSECT;
	}
	return(result);
}

/*int Frustum::BoxInFrustum(const AABB &box)
{
	int result = INSIDE;

	for (int i = 0; i < 6; i++)
	{
		if (pl[i].Distance(box.GetVertexPositive(pl[i].GetNormal())) < 0)
			return OUTSIDE;
		else if (pl[i].Distance(box.GetVertexNegative(pl[i].GetNormal())) < 0)
			result = INTERSECT;
	}
	return(result);
}
*/
