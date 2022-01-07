#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "Traits.h"
#include "ShaderFX.h"
#include "Camera.h"
#include "MatrixInput.h"
#include "Renderer.h"
#include <vector>

class Renderable {
public:
	Renderable(const Shader* shd, const Mesh* mesh, const SubMesh* submesh, 
		const ITransformable* obj, const Camera* cam, const MaterialSetting* setting):
		buffer(mesh), indexData(submesh), shader(shd), object(obj), camera(cam),
		setting(setting)
	{
	}

	~Renderable() {
		inputs.clear();
	}

	Renderable& addInput(const ShaderInput* si) {
		inputs.push_back(si);
		return *this;
	}

	void draw(GLenum mode = GL_TRIANGLES) const;

	const Mesh* buffer;			// vbo
	const SubMesh* indexData;		// ibo
	const Shader* shader;			// shader
	std::vector<const ShaderInput*> inputs;		
	const ITransformable* object;
	const Camera* camera;
	const MaterialSetting* setting;
};
