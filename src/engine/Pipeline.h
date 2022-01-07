#pragma once
#include "SceneGraph.h"
#include "Shader.h"
#include "render_enums.h"

class Renderer;
class AbstractRenderObject;

/// <summary>
/// an abstract pipeline. a pipeline also function as shader input, as it can set data for
/// a particular shader that is being activated
/// </summary>
class Pipeline : public ShaderInput {
public:
	Pipeline(PipelineId _id): renderer(nullptr), p_id(_id) {}
	virtual ~Pipeline() {}

	virtual void draw(const SceneGraph* scene, float dt) = 0;
	virtual void draw(const AbstractRenderObject* obj) = 0;	// can it?
	virtual void printDebug() const = 0;
	virtual void drawDebugImGui() const = 0;

	PipelineId p_id;	// pipeline id?
	// who's our renderer?
	Renderer* renderer;

	// Inherited via ShaderInput
	virtual void setupData(const Shader* shd) const override;
};