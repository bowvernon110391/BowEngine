#pragma once

#include <vector>
#include "Resource.h"

class Shader;
class ShaderInput;
template<typename Resource>
class ResourceManager;
/// <summary>
/// just a holder for a shader + shader data
/// </summary>
class Material : public Resource {
public:
	Material() { sh = nullptr; shData = nullptr; }
	Material(Shader* s, ShaderInput* sd) {
		sh = s;
		shData = sd;
	}

	virtual const char* type() { return "MATERIAL"; }

	Material* withShader(Shader* s) { sh = s; return this; }
	Material* withData(ShaderInput* sd) { shData = sd; return this; }

	Shader* sh;
	ShaderInput* shData;
};