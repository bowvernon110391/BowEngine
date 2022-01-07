#pragma once
#include "Shader.h"
#include "Camera.h"
#include "Traits.h"

class MatrixInput : public ShaderInput {
public:
	MatrixInput(const Camera* cam = nullptr, const ITransformable* obj = nullptr) :
		cam(cam), mdl(obj)
	{}

	const Camera* cam;
	const ITransformable* mdl;

	// Inherited via ShaderInput
	virtual void setupData(const Shader* shd) const override;
};