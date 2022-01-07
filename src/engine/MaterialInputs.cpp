#include "MaterialInputs.h"
#include <SDL.h>

MaterialInput::MaterialInput():
	texture_used(0)
{
}

MaterialInput::~MaterialInput()
{
}

MaterialInput* MaterialInput::addTexture(const Texture2D* tex)
{
	// make sure texture_used is < 8
	SDL_assert(texture_used < 8);

	ShaderParam p(Shader::UniformLoc::texture0 + (texture_used++), ShaderParam::Type::T_TEXTURE );
	p.value.texture = (Texture2D*)tex;

	// add to slot
	inputs.push_back(p);
	// add to texture_inputs (for special processing?)
	texture_inputs.push_back(tex);

	return this;
}

MaterialInput* MaterialInput::addInput1f(int u_id, float v)
{
	ShaderParam p(u_id, ShaderParam::Type::T_1F);
	p.value.constant = v;
	inputs.push_back(p);
	return this;
}

MaterialInput* MaterialInput::addInput2f(int u_id, const glm::vec2& v)
{
	ShaderParam p(u_id, ShaderParam::Type::T_2F);
	p.value.vec2 = v;
	inputs.push_back(p);
	return this;
}

MaterialInput* MaterialInput::addInput3f(int u_id, const glm::vec3& v)
{
	ShaderParam p(u_id, ShaderParam::Type::T_3F);
	p.value.vec3 = v;
	inputs.push_back(p);
	return this;
}

MaterialInput* MaterialInput::addInput4f(int u_id, const glm::vec4& v)
{
	ShaderParam p(u_id, ShaderParam::Type::T_4F);
	p.value.vec4 = v;
	inputs.push_back(p);
	return this;
}

void MaterialInput::setupData(const Shader* shd) const
{
#ifdef _DEBUG_MATERIAL_INPUT
	printf("MATERIAL_INPUT(%X): SETUP_SHADER START\n", this);
#endif // _DEBUG

	int texture_slot = 0;
	int u_loc;
	// just iterate all over the param, and send value
	for (const ShaderParam& p : inputs) {
		// skip invalid uniforms
		u_loc = shd->getUniformLocation(p.uniform_id);
		if (u_loc < 0)
			continue;

		// special treatment for textures
		switch(p.type) {
		case ShaderParam::Type::T_TEXTURE:
			texture_slot = (p.uniform_id - Shader::UniformLoc::texture0);
			glActiveTexture(GL_TEXTURE0 + texture_slot);
			// bind it
			p.value.texture->bind();
			// set its uniform
			glUniform1i(u_loc, texture_slot);
#ifdef _DEBUG_MATERIAL_INPUT
			printf("Material_input: uniform(%d), BINDING TEXTURE(%X) @ SLOT(%d)\n", u_loc, p.value.texture, texture_slot);
#endif // _DEBUG

			break;
		case ShaderParam::Type::T_1F:
			glUniform1f(u_loc, p.value.constant);
			break;
		case ShaderParam::Type::T_2F:
			glUniform2fv(u_loc, 1, glm::value_ptr(p.value.vec2));
			break;
		case ShaderParam::Type::T_3F:
			glUniform3fv(u_loc, 1, glm::value_ptr(p.value.vec3));
			break;
		case ShaderParam::Type::T_4F:
			glUniform3fv(u_loc, 1, glm::value_ptr(p.value.vec4));
			break;
		}

		++texture_slot;
	}

#ifdef _DEBUG_MATERIAL_INPUT
	printf("MATERIAL_INPUT(%X): SETUP_SHADER END\n", this);
#endif // _DEBUG
}

const char* MaterialInput::type()
{
	return "MATERIAL_INPUT";
}
