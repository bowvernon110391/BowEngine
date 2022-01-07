#include "Camera.h"
#include "AABB.h"

Frustum::TestResult Frustum::testAABB(const AABB* bbox) const
{
	// make center point
	glm::vec3 extent = (bbox->max - bbox->min) * 0.5f;
	glm::vec3 center = (bbox->min + extent);

	// make variation
	const glm::vec3 t[4] = {
		glm::vec3(extent),
		glm::vec3(extent.x, -extent.y, -extent.z),
		glm::vec3(-extent.x,  extent.y, -extent.z),
		glm::vec3(-extent.x, -extent.y, extent.z)
	};

	int pass = 0;
	for (int i = 0; i < 6; i++) {
		float ext_dist = 0.0f;
		for (int v = 0; v < 4; v++) {
			float ed = fabs(t[v].x * planes[i].x + t[v].y * planes[i].y + t[v].z * planes[i].z);
			if (v == 0 || ed > ext_dist)
				ext_dist = ed;
		}

		// compute distance?
		float d = planes[i].x * center.x + planes[i].y * center.y + planes[i].z * center.z + planes[i].w;
		if (d < -ext_dist) {
			return TestResult::FULLY_OUT;
		}
		else if (d > ext_dist)
			pass++;
	}

	return pass == 6 ? TestResult::FULLY_IN : TestResult::INTERSECT;
}

Frustum::TestResult Frustum::testPoint(const glm::vec3& p) const
{
	for (int i = 0; i < 6; i++) {
		float d = planes[i].x * p.x + planes[i].y * p.y + planes[i].z * p.z + planes[i].w;
		if (d < 0.0f)
			return TestResult::FULLY_OUT;
	}
	return TestResult::FULLY_IN;
}

Frustum::TestResult Frustum::testSphere(const glm::vec3& p, float r) const
{
	int pass = 0;
	for (int i = 0; i < 6; i++) {
		float d = (planes[i].x * p.x + planes[i].y * p.y + planes[i].z * p.z + planes[i].w);
		// if even fails at one of this, it's out
		if (d < -r)
			return TestResult::FULLY_OUT;
		else if (d > r)
			pass++;
	}
	return pass == 6 ? TestResult::FULLY_IN : TestResult::INTERSECT;
}

void Camera::updateFrustum()
{
	// clip to world matrices
	glm::mat4 c = proj_cached * view_cached;
    // set the 6 planes
	// LEFT
	glm::vec4& left = frustum.planes[0];
	left = glm::vec4(c[0][3] + c[0][0], c[1][3] + c[1][0], c[2][3] + c[2][0], c[3][3] + c[3][0]);
	// RIGHT
	glm::vec4& right = frustum.planes[1];
	right = glm::vec4(c[0][3] - c[0][0], c[1][3] - c[1][0], c[2][3] - c[2][0], c[3][3] - c[3][0]);
	// BOTTOM
	glm::vec4& bottom = frustum.planes[2];
	bottom = glm::vec4(c[0][3] + c[0][1], c[1][3] + c[1][1], c[2][3] + c[2][1], c[3][3] + c[3][1]);
	// TOP
	glm::vec4& top = frustum.planes[3];
	top = glm::vec4(c[0][3] - c[0][1], c[1][3] - c[1][1], c[2][3] - c[2][1], c[3][3] - c[3][1]);
	// NEAR
	glm::vec4& near = frustum.planes[4];
	near = glm::vec4(c[0][3] + c[0][2], c[1][3] + c[1][2], c[2][3] + c[2][2], c[3][3] + c[3][2]);
	// FAR
	glm::vec4& far = frustum.planes[5];
	far = glm::vec4(c[0][3] - c[0][2], c[1][3] - c[1][2], c[2][3] - c[2][2], c[3][3] - c[3][2]);

	// normalize it
	for (int i = 0; i < 6; i++) {
		glm::vec4& p = frustum.planes[i];
		float l = sqrtf(p.x * p.x + p.y * p.y + p.z * p.z);
		float oneOverL = l > 0.0f ? 1.f / l : 1.f;

		p = p * oneOverL;
	}
}

void Camera::updateMatrices()
{
    view_cached = glm::lookAt(eye, target, up);
	if (isPerspective) {
		proj_cached = glm::perspectiveFov(glm::radians(fov), aspect, 1.0f, nearPlane, farPlane);
	}
	else {
		proj_cached = glm::ortho(left, right, bottom, top, nearPlane, farPlane);
	}

	// update frustum too? 
	updateFrustum();
}
