#pragma once
#include "Pipeline.h"

class UnlitPipeline : public Pipeline {
public:
	UnlitPipeline();
	virtual ~UnlitPipeline();

	// Inherited via Pipeline
	virtual void draw(const SceneGraph* scene, float dt) override;
	virtual void draw(const AbstractRenderObject* obj) override;
	virtual void printDebug() const override;

	// this can be accessed by the next pipeline...
	std::vector<const SceneObject*> frustum_culled;

	// Inherited via Pipeline
	virtual void drawDebugImGui() const override;
};