#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "SceneGraph.h"
#include "Pipeline.h"
#include "render_enums.h"
#include "ShaderInstanceManager.h"

struct MaterialSetting;
/// <summary>
/// This is our renderer class
/// </summary>
/// 
class Renderer {
public:
	Renderer();
	virtual ~Renderer();

	Renderer* useCamera(Camera* c);

	// viewport related
	Renderer* setViewport(int x, int y, int w, int h);
	int* getViewport() { return &viewport[0]; }

	// active camera (UGH!!)
	Camera* getCamera() const { return m_camera;  }

	// pipeline related
	Renderer* setPipeline(PipelineId id, Pipeline* p);
	const Pipeline* getPipeline(PipelineId id) const { return pipelines[id]; }

	// debugging related
	Shader* getDebugShader() { return debugShader; }
	void generateDebugData(const AABB& b);
	int getDebugVBO() { return vboDebug; }
	int getDebugIBO() { return iboDebug; }

	// get shader manager
	ShaderInstanceManager* getShaderManager() const { return shdMgr; }

	// big time
	void draw(const SceneGraph* scene, float dt);

	// update time
	void updateTime(float dt);

	// usable by everyone?
	static void setupVertexState(const Shader* shd);
	static void setupVertexArray(const Shader* shd, const Mesh* m);
	static void setupGLState(const MaterialSetting* set);
	static const MaterialSetting* getDefaultLightingGLState() { return &defaultLightingGLState; }

	bool drawDebug;
	glm::vec4 debugColor;
protected:
	friend class Pipeline;

	// default ogl state for lighting
	static MaterialSetting defaultLightingGLState;

	// shader instance manager
	ShaderInstanceManager* shdMgr;

	// the pipelines
	Pipeline* pipelines[PipelineId::TOTAL_PIPELINE];

	Camera *m_camera;

	int viewport[4];
	int lastAttribFlags;

	// for debugging?
	uint32_t vboDebug, iboDebug;
	std::vector<float> vbDebug;
	std::vector<unsigned short> ibDebug;
	Shader* debugShader;
	float tick_time, render_time;

	void initDebugData();
	void initSharedData();
	void destroyDebugData();
	void initPipelines();
	void destroyPipelines();
};