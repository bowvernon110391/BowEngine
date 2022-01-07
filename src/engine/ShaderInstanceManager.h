#pragma once
#include <unordered_map>
#include <string>
#include "ShaderSource.h"
#include "Shader.h"

class ShaderInstanceManager {
public:
	ShaderInstanceManager();
	~ShaderInstanceManager();

	const Shader* getShader(const ShaderKey& key);
	const Shader* getFirstShader() const {
		return shaders.begin()->second;
	}

	void printDebug()const;

protected:
	std::unordered_map<ShaderKey, Shader*> shaders;
};