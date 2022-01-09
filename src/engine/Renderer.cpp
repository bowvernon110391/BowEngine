#include "Renderer.h"
#include "Mesh.h"
#include "ShaderFX.h"

#include "UnlitPipeline.h"
//#define DEBUG_VA

// shared
MaterialSetting Renderer::defaultLightingGLState = MaterialSetting()
	.setOpacityType(OpacityType::ALPHA_BLEND)
	.setBlendMode(BlendMode::ADDITIVE);

Renderer::Renderer() {
	m_camera = 0;
	lastAttribFlags = 0;

	initDebugData();
	initPipelines();

	shdMgr = new ShaderInstanceManager();
}

Renderer::~Renderer() {
	if (m_camera) {
		delete m_camera;
		m_camera = 0;
	}

	if (shdMgr) {
		shdMgr->printDebug();
		delete shdMgr;
	}

	destroyDebugData();
	destroyPipelines();
}

void Renderer::initDebugData() {
	// set color
	debugColor = glm::vec4(1.0, 0.0, 0.0, 1.0);
	// generate cube, with size 1 perhaps?
	// genereat buffers
	glGenBuffers(1, &vboDebug);
	glGenBuffers(1, &iboDebug);

	glBindBuffer(GL_ARRAY_BUFFER, vboDebug);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboDebug);

	// fill our vb
	vbDebug.clear();
	ibDebug.clear();

	float half = 0.5f;
	float verts[] = {
		// xyz uv
		// front
		-half, -half, half,	// 0
		 half, -half, half,	// 1
		 half,  half, half,	// 2
		-half,  half, half,	// 3

		// back
		 half, -half,-half,	// 4
		-half, -half,-half,	// 5
		-half,  half,-half,	// 6
		 half,  half,-half,	// 7
	};

	// index em (lines)
	uint16_t indices[] = {
		0,1, 1,2, 2,3, 3,0,
		4,5, 5,6, 6,7, 7,4,
		1,4, 2,7, 6,3, 5,0
	};

	vbDebug.insert(vbDebug.end(), &verts[0], &verts[24]);
	ibDebug.insert(ibDebug.end(), &indices[0], &indices[24]);

	glBufferData(GL_ARRAY_BUFFER, vbDebug.size() * sizeof(float), &vbDebug[0], GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ibDebug.size() * sizeof(unsigned short), &ibDebug[0], GL_STATIC_DRAW);

	// init shader
	const char* vs = "\
		attribute vec3 position; \n\
		uniform mat4 m_model_view_projection; \n\
		void main() { \n\
			gl_Position = m_model_view_projection * vec4(position, 1.0); \n\
		} \n\
		";

	const char* fs = "\
	 	#ifdef GL_ES	\n\
		precision mediump float;	\n\
		#endif	\n\
		uniform vec4 material_diffuse; \n\
		void main() { \n\
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n\
		} \n\
		";

	debugShader = Shader::fromMemory(vs, strlen(vs), fs, strlen(fs));
}

void Renderer::initSharedData()
{
}

Renderer* Renderer::setPipeline(PipelineId id, Pipeline* p)
{
	SDL_assert(id < PipelineId::TOTAL_PIPELINE);
	// set relation
	pipelines[id] = p;
	p->renderer = this;
	return this;
}

void Renderer::generateDebugData(const AABB& b) {
	// just recreate the vertbuffer (make sure it contains space for 8x vec3)
	assert(vbDebug.size() == 24);

	const glm::vec3& min = b.min;
	const glm::vec3& max = b.max;

	// make sure to follow the format
	vbDebug[0] = min.x;	vbDebug[1] = min.y; vbDebug[2] = max.z;
	vbDebug[3] = max.x;	vbDebug[4] = min.y; vbDebug[5] = max.z;
	vbDebug[6] = max.x;	vbDebug[7] = max.y; vbDebug[8] = max.z;
	vbDebug[9] = min.x;	vbDebug[10] = max.y; vbDebug[11] = max.z;
	
	vbDebug[12] = max.x;	vbDebug[13] = min.y; vbDebug[14] = min.z;
	vbDebug[15] = min.x;	vbDebug[16] = min.y; vbDebug[17] = min.z;
	vbDebug[18] = min.x;	vbDebug[19] = max.y; vbDebug[20] = min.z;
	vbDebug[21] = max.x;	vbDebug[22] = max.y; vbDebug[23] = min.z;

	glBindBuffer(GL_ARRAY_BUFFER, vboDebug);
	glBufferData(GL_ARRAY_BUFFER, vbDebug.size() * sizeof(float), &vbDebug[0], GL_STREAM_DRAW);
}

void Renderer::destroyDebugData() {
	vbDebug.clear();
	ibDebug.clear();
	glDeleteBuffers(1, &vboDebug);
	glDeleteBuffers(1, &iboDebug);
	delete debugShader;
}

void Renderer::initPipelines()
{
	for (int i = 0; i < PipelineId::TOTAL_PIPELINE; i++) {
		pipelines[i] = nullptr;
	}

	setPipeline(PipelineId::P_DRAW_UNLIT, new UnlitPipeline());
}

void Renderer::destroyPipelines()
{
	for (int i = 0; i < PipelineId::TOTAL_PIPELINE; i++) {
		if (pipelines[i]) {
			delete pipelines[i];
			pipelines[i] = nullptr;
		}
	}
}

Renderer* Renderer::useCamera(Camera* cam) {
	if (m_camera && cam) {
		delete m_camera;
	}
	m_camera = cam;
	return this;
}

Renderer* Renderer::setViewport(int x, int y, int w, int h) {
	viewport[0] = x;
	viewport[1] = y;
	viewport[2] = w;
	viewport[3] = h;

	if (m_camera)
		m_camera->setAspect((float)w / (float)h);

	glViewport(x, y, w, h);

	return this;
}

void Renderer::draw(const SceneGraph* scene, float dt)
{
	// update render time
	render_time = tick_time + dt;

	// update camera?
	if (m_camera) {
		m_camera->updateMatrices();
		m_camera->updateFrustum();
	}

	// just let pipeline do its job?
	for (int i = 0; i < PipelineId::TOTAL_PIPELINE; i++) {
		if (pipelines[i])
			pipelines[i]->draw(scene, dt);
	}
}

void Renderer::updateTime(float dt)
{
	tick_time += dt;
}

void Renderer::setupVertexState(const Shader* shd) {
	// disable all shits, then enable the relevant
	for (int i = Shader::AttribLoc::position; i < Shader::AttribLoc::custom_attribute; i++) {
		if (shd->attributeFlags & (1 << i)) {
			glEnableVertexAttribArray(shd->getAttribLocation(i));
		}
	}
}


void Renderer::setupVertexArray(const Shader* shd, const Mesh* m)
{
#ifdef DEBUG_VA
	SDL_Log("VERTEX_ARRAY: shd_attrib_flags(%d), mesh_vtx_format(%d)\n", shd->attributeFlags, m->vertexFormat);
#endif // DEBUG_VA

	// now just call glVertexAttribArray on all possible shiets
	size_t offset = 0;
	int a_loc = -1;

	// gotta follow mesh vertex format
	// position
	a_loc = shd->getAttribLocation(Shader::AttribLoc::position);
	if ((m->vertexFormat & VF_XYZ) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: POSITION @ %d bytes\n", a_loc, offset);
#endif // DEBUG

		glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_XYZ)) * sizeof(float) * 3;

	// normal
	a_loc = shd->getAttribLocation(Shader::AttribLoc::normal);
	if ((m->vertexFormat & VF_NORMAL) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: NORMAL @ %d bytes\n", a_loc, offset);
#endif // DEBUG
		
		glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_NORMAL) >> 1) * sizeof(float) * 3;

	// uv
	a_loc = shd->getAttribLocation(Shader::AttribLoc::uv);
	if ((m->vertexFormat & VF_UV) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: UV @ %d bytes\n", a_loc, offset);
#endif // DEBUG

		glVertexAttribPointer(a_loc, 2, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_UV) >> 2) * sizeof(float) * 2;

	// tangent, bitangent
	if (m->vertexFormat & VF_TANGENT) {
		// tangent first
		a_loc = shd->getAttribLocation(Shader::AttribLoc::tangent);
		if (a_loc >= 0) {
#ifdef DEBUG_VA
			SDL_Log("VERTEX_ARRAY[%d]: TANGENT @ %d bytes\n", a_loc, offset);
#endif // DEBUG
			glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)offset);
		}

		// then bitangent
		a_loc = shd->getAttribLocation(Shader::AttribLoc::bitangent);
		if (a_loc >= 0) {
#ifdef DEBUG_VA
			SDL_Log("VERTEX_ARRAY[%d]: BITANGENT @ %d bytes\n", a_loc, offset+sizeof(float)*3);
#endif // DEBUG
			glVertexAttribPointer(a_loc, 3, GL_FLOAT, false, m->strideLength, (void*)(offset + sizeof(float) * 3));
		}
	}
	offset += ((m->vertexFormat & VF_TANGENT) >> 3) * sizeof(float) * 6;

	// second uv
	a_loc = shd->getAttribLocation(Shader::AttribLoc::uv2);
	if ((m->vertexFormat & VF_UV2) && a_loc >= 0) {
#ifdef DEBUG_VA
		SDL_Log("VERTEX_ARRAY[%d]: UV2 @ %d bytes\n", a_loc, offset);
#endif // DEBUG
		glVertexAttribPointer(a_loc, 2, GL_FLOAT, false, m->strideLength, (void*)offset);
	}
	offset += ((m->vertexFormat & VF_UV2) >> 4) * sizeof(float) * 2;
}

void Renderer::setupGLState(const MaterialSetting* set)
{
	// what is the state?
	if (set->depthTest) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	glDepthMask(set->depthWrite);

	if (set->ot == OpacityType::ALPHA_BLEND) {
		glEnable(GL_BLEND);
		switch (set->bm) {
		case BlendMode::NORMAL:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BlendMode::ADDITIVE:
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		}
	}
	else {
		glDisable(GL_BLEND);
	}
}
