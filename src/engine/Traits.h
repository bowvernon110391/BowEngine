#pragma once
#include "AABB.h"
#include <glm/glm.hpp>

class IBoundable {
public:
	virtual AABB getBoundingBox() = 0;
};

class ITransformable {
public:
	virtual bool getTransform(glm::mat4* m) const = 0;
};