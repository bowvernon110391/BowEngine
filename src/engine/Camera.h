#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class AABB;
// a frustum class
class Frustum {
public:
	enum TestResult {
		FULLY_IN,
		FULLY_OUT,
		INTERSECT
	};

	Frustum() {}
	~Frustum() {}

	// various testing
	TestResult testAABB(const AABB* bbox) const;
	TestResult testPoint(const glm::vec3& p) const;
	TestResult testSphere(const glm::vec3& p, float r) const;

	// helper
	static const char* getPlaneName(int id) {
		switch (id) {
		case 0:
			return "LEFT";
		case 1:
			return "RIGHT";
		case 2:
			return "BOTTOM";
		case 3:
			return "TOP";
		case 4:
			return "NEAR";
		case 5:
			return "FAR";
		}
		return "UNKNOWN";
	}

	// contain 6 planes
	glm::vec4 planes[6];
};

class Camera {
public:
	Camera() {
		up = glm::vec3(0, 1, 0);
		target = eye + glm::vec3(0, 0, -1);
		fov = 80.0f;	// default to 80 deg
		aspect = 1.0f;
		isPerspective = true;
		nearPlane = 0.2f;
		farPlane = 100.0f;
	}

	virtual const glm::mat4& getViewMatrix() const {
		return view_cached;
	}

	virtual const glm::mat4& getProjectionMatrix() const {
		return proj_cached;
	}

	Camera* setAspect(float a) { aspect = a; return this; }
	Camera* usePerspective(bool u) { isPerspective = u; return this; }
	Camera* setPosition(const glm::vec3 v) { eye = v; return this; }
	Camera* setTarget(const glm::vec3 v) { target = v; return this; }
	Camera* setUp(const glm::vec3 v) { up = v; return this; }
	Camera* setFov(const float& f) { fov = f; return this;  }
	Camera* setClipDistance(float n, float f) {
		nearPlane = n; 
		farPlane = f;
		return this;
	}

	void updateFrustum();
	void updateMatrices();

	const glm::vec3& getPosition() const { return eye;  }
	const glm::vec3& getTarget() const { return target;  }
	const glm::vec3& getUp() const { return up;  }
	glm::vec3 getDir() const {
		return target - eye;
	}

	const Frustum* getFrustum() const { return &frustum;  }

protected:
	// what does a camera have?
	glm::vec3 eye, target, up;
	// is it perspective camera?
	bool isPerspective;
	// for perspective
	float fov;
	float aspect;
	float nearPlane, farPlane;
	// for orthogonal
	float left, right, top, bottom;
	// the frustum
	Frustum frustum;
	// cached view matrix
	glm::mat4 view_cached;
	glm::mat4 proj_cached;
};