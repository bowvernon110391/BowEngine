#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>

class AABB {
public: 
	AABB() {
		min = glm::vec3(0.0f);
		max = glm::vec3(0.0f);
	}

	AABB(const glm::vec3& bMin, const glm::vec3& bMax)
		: min(bMin), max(bMax) {
		// nothing
	}

	AABB(const AABB& b) : 
		AABB(b.min, b.max)
	{}

	// expand bounded area
	void expand(const AABB& b) {
		min = glm::min(min, b.min);
		max = glm::max(max, b.max);
	}

	// expand by vector
	void expand(const glm::vec3& v) {
		if (v.x > 0) max.x += v.x; else min.x += v.x;
		if (v.y > 0) max.y += v.y; else min.y += v.y;
		if (v.z > 0) max.z += v.z; else min.z += v.z;
	}

	// expand by a constant value (fatten it)
	void expand(float f) {
		min += glm::vec3(-f, -f, -f);
		max += glm::vec3(f, f, f);
	}

	// surface area of AABB
	float area() const {
		glm::vec3 d = max - min;
		return 2.0f * (d.x * d.y + d.y * d.z + d.x * d.z);
	}

	// volume of AABB
	float volume() const {
		glm::vec3 d = max - min;
		return (d.x * d.y * d.z);
	}

	// get transformed aabb
	AABB transform(const glm::vec3& pos, const glm::quat& rot) const {
		// compute transformed center?
		glm::vec3 center = ((min + max) * 0.5f);
		glm::vec4 t_center = glm::vec4(center, 1.0f);
		t_center = glm::rotate(rot, t_center);
		center = pos + glm::vec3(t_center);

		// now compute half extent in local space, and rotate it?
		glm::vec3 e = (max - min) * 0.5f;

		const glm::vec3 modifiers[] = {
			glm::vec3(1,1,1),
			glm::vec3(-1,-1,1),
			glm::vec3(1,-1,-1),
			glm::vec3(-1,1,-1),
		};

		glm::vec3 newMin, newMax;

		// init extent
		glm::vec4 re = glm::rotate(rot, glm::vec4(e, 1.0f));
		newMin = glm::vec3(glm::min(re.x, -re.x), glm::min(re.y, -re.y), glm::min(re.z, -re.z));
		newMax = glm::vec3(glm::max(re.x, -re.x), glm::max(re.y, -re.y), glm::max(re.z, -re.z));

		for (int i = 1; i < 4; i++) {
			re = glm::rotate(rot, glm::vec4(e * modifiers[i], 1.0f));

			newMin.x = glm::min(glm::min(re.x, -re.x), newMin.x);
			newMin.y = glm::min(glm::min(re.y, -re.y), newMin.y);
			newMin.z = glm::min(glm::min(re.z, -re.z), newMin.z);

			newMax.x = glm::max(glm::max(re.x, -re.x), newMax.x);
			newMax.y = glm::max(glm::max(re.y, -re.y), newMax.y);
			newMax.z = glm::max(glm::max(re.z, -re.z), newMax.z);
		}

		// offset by the right center
		newMin += center;
		newMax += center;

		return AABB(newMin, newMax);
	}

	AABB transform(const glm::mat4& mat) const {
		glm::mat3 m_rot = glm::mat3(mat);
		// compute transformed center?
		glm::vec3 center = ((min + max) * 0.5f);
		glm::vec4 t_center = glm::vec4(center, 1.0f);

		// transform center
		center = glm::vec3(mat * glm::vec4(center, 1.0));

		// now compute half extent in local space, and rotate it?
		glm::vec3 e = (max - min) * 0.5f;

		const glm::vec3 modifiers[] = {
			glm::vec3(1,1,1),
			glm::vec3(-1,-1,1),
			glm::vec3(1,-1,-1),
			glm::vec3(-1,1,-1),
		};

		glm::vec3 newMin, newMax;

		// init extent
		glm::vec3 re = m_rot * e; //glm::rotate(rot, glm::vec4(e, 1.0f));
		newMin = glm::vec3(glm::min(re.x, -re.x), glm::min(re.y, -re.y), glm::min(re.z, -re.z));
		newMax = glm::vec3(glm::max(re.x, -re.x), glm::max(re.y, -re.y), glm::max(re.z, -re.z));

		for (int i = 1; i < 4; i++) {
			re = m_rot * (e * modifiers[i]); /*glm::rotate(rot, glm::vec4(e * modifiers[i], 1.0f));*/

			newMin.x = glm::min(glm::min(re.x, -re.x), newMin.x);
			newMin.y = glm::min(glm::min(re.y, -re.y), newMin.y);
			newMin.z = glm::min(glm::min(re.z, -re.z), newMin.z);

			newMax.x = glm::max(glm::max(re.x, -re.x), newMax.x);
			newMax.y = glm::max(glm::max(re.y, -re.y), newMax.y);
			newMax.z = glm::max(glm::max(re.z, -re.z), newMax.z);
		}

		// offset by the right center
		newMin += center;
		newMax += center;

		return AABB(newMin, newMax);
	}

	// return union of two aabb
	static AABB combined(const AABB& a, const AABB& b) {
		return AABB(
			glm::min(a.min, b.min),
			glm::max(a.max, b.max)
		);
	}

	// boolean check
	static bool intersect(const AABB& a, const AABB& b) {
		return (
			a.min.x < b.max.x && 
			a.min.y < b.max.y && 
			a.min.z < b.max.z
			&&
			b.min.x < a.max.x && 
			b.min.y < a.max.y && 
			b.min.z < a.max.z
			);
	}

	// containment check. meaning b's bound is still within a's containment
	static bool contain(const AABB& a, const AABB& b) {
		return (b.min.x >= a.min.x && b.min.y >= a.min.y && b.min.z >= a.min.z)
			&& (b.max.x <= a.max.x && b.max.y <= a.max.y && b.max.z <= a.max.z);
	}

	glm::vec3 min, max;
};