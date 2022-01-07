#pragma once
#include "Texture2d.h"
#include "Resource.h"
#include "Shader.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Shader;
class RenderPass;

// a struct that hold a single input
struct ShaderParam {
	enum Type {
		T_1F, 
		T_2F,
		T_3F,
		T_4F,
		T_TEXTURE
	};

	ShaderParam(int u_id, Type t) {
		uniform_id = u_id;	// invalid uniform initially?
		type = t;
	}

	union {
		float constant;
		glm::vec2 vec2;
		glm::vec3 vec3;
		glm::vec4 vec4;
		Texture2D* texture;
		void* data;
	} value;
	int uniform_id;
	Type type;
};


/// <summary>
/// Base data for material input (aka Shader parameter)
/// </summary>
class MaterialInput : public ShaderInput {
public:
	MaterialInput();
	virtual ~MaterialInput();

	MaterialInput* addTexture(const Texture2D* tex);
	MaterialInput* addInput1f(int u_id, float v);
	MaterialInput* addInput2f(int u_id, const glm::vec2& v);
	MaterialInput* addInput3f(int u_id, const glm::vec3& v);
	MaterialInput* addInput4f(int u_id, const glm::vec4& v);
	virtual void setupData(const Shader* shd) const;

	std::vector<ShaderParam> inputs;
	std::vector<const Texture2D*> texture_inputs;
	int texture_used;
	// Inherited via Resource
	virtual const char* type() override;
	// 0 initially, increasing as more texture is added
};