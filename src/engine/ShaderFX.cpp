#include "ShaderFX.h"

ShaderFX::~ShaderFX() {
	for (int i = 0; i < PipelineId::TOTAL_PIPELINE; i++) {
		if (techniques[i])
			delete techniques[i];
		techniques[i] = nullptr;
	}
}

const char* ShaderFX::type()
{
    return "SHADER_EFFECT";
}

ShaderTechnique::~ShaderTechnique() {
	for (RenderPass* p : passes) {
		if (p)
			delete p;
	}
	passes.clear();
}

void ShaderTechnique::getPassesByLightingStage(LightingStage ls, std::vector<RenderPass*>& bucket) {
	// now, grab any matching pass?
	for (RenderPass* pass : passes) {
		if (ls & pass->getLightingStage()) {
			// add it
			bucket.push_back(pass);
		}
	}
}
