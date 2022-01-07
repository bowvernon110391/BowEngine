#include "UnlitPipeline.h"
#include "SceneGraph.h"
#include "Renderer.h"
#include "ShaderFX.h"
#include "AbstractRenderObject.h"
#include "Renderable.h"
#include "MatrixInput.h"
#include <vector>
#include "../imgui/imgui.h"

//#define DEBUG_PIPELINE

UnlitPipeline::UnlitPipeline():
	Pipeline(PipelineId::P_DRAW_UNLIT)
{
}

UnlitPipeline::~UnlitPipeline()
{
}

void UnlitPipeline::draw(const SceneGraph* scene, float dt)
{
#ifdef DEBUG_PIPELINE
	printf("--------------------PIPELINE START-------------------------\n");
#endif // DEBUG_PIPELINE

	// set states
	glEnable(GL_DEPTH_TEST);

	// clear
	frustum_culled.clear();

	// grab frustum
	const Frustum* f = renderer->getCamera()->getFrustum();

	// clip them, renderables, static and dynamics
	scene->clip(f, SceneGraph::CLIP_RENDERABLES|SceneGraph::CLIP_STATIC|SceneGraph::CLIP_DYNAMIC, frustum_culled);

#ifdef DEBUG_PIPELINE
	SDL_Log("UNLIT_PIPELINE: frustum_culled(%d)\n", frustum_culled.size());
#endif // _DEBUG

	for (const SceneObject* o : frustum_culled) {
		const AbstractRenderObject* ro = (const AbstractRenderObject*)o;
		// for now, just draw em
		draw(ro);
	}

#ifdef DEBUG_PIPELINE
	printf("+++++++++++++++++++++++PIPELINE END+++++++++++++++++++++++++++++\n");
#endif // DEBUG_PIPELINE
}

void UnlitPipeline::draw(const AbstractRenderObject* obj)
{
#ifdef DEBUG_PIPELINE
	printf("*********************BEGIN_DRAW_OBJECT**********************\n");
#endif
	// grab shader instancer
	ShaderInstanceManager* instancer = renderer->getShaderManager();
	// okay, grab mesh ref?
	const Mesh* mesh = obj->getMeshLayout();
	
	// okay, draw them
	const int drawable_count = obj->effects.size() & mesh->subMeshes.size();

#ifdef DEBUG_PIPELINE
	printf("Drawing obj(%X), drawable_count(%d)\n", obj, drawable_count);
#endif // DEBUG_PIPELINE


	for (int i = 0; i < drawable_count; i++) {
		// teh effect
		ShaderFX* fx = obj->effects[i];

		// for the draw call
		const SubMesh* sm = (const SubMesh*)&mesh->subMeshes[i];

		// if the effect is not defined in this pipeline,
		// skip it
		if (!fx->hasPipeline(p_id)) {
#ifdef DEBUG_PIPELINE
			printf("Submesh[%d] UNLIT undefined!\n");
#endif // DEBUG_PIPELINE

			continue;
		}

		// yep, render them
		// grab the technique
		ShaderTechnique* tech = fx->techniques[p_id];
		// execute every pass
		const std::vector<RenderPass*> passes = tech->getPasses();

#ifdef DEBUG_PIPELINE
		printf("UNLIT submesh(%d) tech(%X) pass_count(%d)\n", i, tech, passes.size());
#endif // DEBUG_PIPELINE

		int pass_id = 0;
		for (const RenderPass* p : passes) {
#ifdef DEBUG_PIPELINE
			printf("============================================\n");
			printf("BEGIN_DRAWING_PASS %d\n", pass_id);
#endif // DEBUG_PIPELINE
			++pass_id;

			// draw them
			ShaderKey k(p->getShaderSource(), LightType::UNLIT, p->getMaterialSetting().ot, obj->getGeometryType());
			// grab shader
			const Shader* shd = instancer->getShader(k);
			// build renderable?
			Renderable r(
				shd, mesh, sm, obj, renderer->getCamera(), &p->getMaterialSetting()
			);
			// assign inputs?
			r.addInput(this);					// from pipeline
			r.addInput(p->getShaderInput());	// from pass/material input
			r.addInput(obj);					// from object

			// draw em
			r.draw();
			
#ifdef DEBUG_PIPELINE
			printf("END_DRAWING_PASS\n");
			printf("-------------------------------------------\n");
#endif // DEBUG_PIPELINE
		}
	}

#ifdef DEBUG_PIPELINE
	printf("*********************END_DRAW_OBJECT**********************\n");
#endif
}

void UnlitPipeline::printDebug() const
{
}

void UnlitPipeline::drawDebugImGui() const
{
	// put it in a child or smth
	ImGui::Text("VISIBLE_UNLIT: %d\n", frustum_culled.size());
}
