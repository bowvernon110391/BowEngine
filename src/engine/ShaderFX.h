#pragma once
#include "render_enums.h"
#include "Resource.h"
#include "Shader.h"
#include "ShaderSource.h"
#include <vector>

class ShaderTechnique;
class RenderPass;

/// <summary>
/// the shader fx, or the material, contains shader and inputs basically
/// </summary>
class ShaderFX : public Resource {
public:
	ShaderFX() {
		// init them all
		for (int i = 0; i < PipelineId::TOTAL_PIPELINE; i++)
			techniques[i] = nullptr;
	}
	virtual ~ShaderFX();

	// Inherited via Resource
	virtual const char* type() override;

	// simple checker
	bool hasPipeline(PipelineId p) const { return techniques[p] != nullptr; }

	// add technique?
	ShaderFX* setTechnique(PipelineId p, ShaderTechnique* tech) {
		techniques[p] = tech;
		return this;
	}

	// made all public for easy access
	ShaderTechnique* techniques[PipelineId::TOTAL_PIPELINE];
};


/// <summary>
/// a technique is collection of several passes
/// </summary>
class ShaderTechnique {
public:
	ShaderTechnique() {}
	virtual ~ShaderTechnique();

	ShaderTechnique* addPass(RenderPass* p) {
		SDL_assert(p != nullptr);
		passes.push_back(p);
		return this;
	}

	const std::vector<RenderPass*> getPasses()const { return passes; }

	// gotta query for a specific pass?
	void getPassesByLightingStage(LightingStage ls, std::vector<RenderPass*>& bucket);

	RenderPass* getDefaultPass() const {
		SDL_assert(passes.size() > 0);
		return passes[0];
	}

private:
	std::vector<RenderPass*> passes;
};

// material setting
struct MaterialSetting {
	MaterialSetting() {
		// default setting
		depthWrite = depthTest = true;
		ot = OpacityType::OPAQUE;
		bm = BlendMode::NORMAL;
	}

	MaterialSetting& setOpacityType(OpacityType o) { ot = o; return *this; }
	MaterialSetting& setBlendMode(BlendMode b) { bm = b; return *this; }
	MaterialSetting& setDepthWrite(bool flag) { depthWrite = flag; return *this; }
	MaterialSetting& setDepthTest(bool flag) { depthTest = flag; return *this; }

	OpacityType ot;		// opaque? alphaclip? alphablend?
	BlendMode bm;		// used for alphablend opacity mode
	bool depthWrite;	// write to depth?
	bool depthTest;		// depth test?
};

/// <summary>
/// a renderpass is a shader (source/prototype), and some inputs
/// </summary>
class RenderPass {
public:
	RenderPass(ShaderSource* src, ShaderInput* si):
		shd_src(src), shd_param(si), ls(LightingStage::NONE)
	{
		SDL_assert(src != nullptr);
		SDL_assert(si != nullptr);
		// default
		mat_setting.ot = OpacityType::OPAQUE;
	}
	virtual ~RenderPass() {}

	RenderPass* setShaderSource(ShaderSource* src) { shd_src = src; return this; }
	RenderPass* setShaderInput(ShaderInput* si) { shd_param = si; return this; }

	RenderPass* setMaterialSetting(const MaterialSetting& s) {
		mat_setting = s;
		return this;
	}
	RenderPass* setLightingStage(LightingStage ls) { this->ls = ls; return this; }

	const ShaderSource* getShaderSource()const { return shd_src; }
	const ShaderInput* getShaderInput()const { return shd_param; }

	const LightingStage& getLightingStage()const { return ls; }
	const MaterialSetting& getMaterialSetting()const { return mat_setting; }
private:
	// got a shader source, a key, and input?
	ShaderSource* shd_src;
	ShaderInput* shd_param;
	MaterialSetting mat_setting;
	LightingStage ls;	// where does this execute (for DRAW_LIT pipeline only)
};