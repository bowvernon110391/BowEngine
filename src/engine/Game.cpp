
#include "Game.h"
#include <math.h>
#include "Helper.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>

// include imgui?
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl.h"
#include "../imgui/imgui_impl_bowie.h"

#include "Renderer.h"
#include "Camera.h"
#include "SceneData.h"
#include "Shader.h"
#include "Mesh.h"
#include "LargeMesh.h"
#include "Material.h"
#include "ResourceManager.h"
#include "AABBTree.h"
#include "ShaderSource.h"
#include "ShaderInstanceManager.h"
#include "SceneGraph.h"
#include "ShaderFX.h"
#include "MaterialInputs.h"
#include "MatrixInput.h"
#include "BufferObjects.h"

#include "StaticMeshObject.h"

// try locally first?
FBO* fbo;
RBO* rbo;
bool mainViewportFocused = false;

Game::Game() {
	cam_horzRot = 0;
	cam_vertRot = -45.0f;
	cam_dist = 5.0f;

	ctxData = new ContextData(this->wndApp, this->glCtx);

	clearColor = ImVec4(0.2f, 0.0f, 0.3f, 1.0f);
}

Game::~Game() {
	delete ctxData;
}

void Game::onInit() {
	// debug print
	//glEnable(GL_MULTISAMPLE);
	srand(SDL_GetTicks());
	initImGui();


	m_renderer = (new Renderer)
		->useCamera((new Camera())
			->setPosition(glm::vec3(0, 7, 5))
			->setTarget(glm::vec3(0))
			->setFov(80.0f)
			->usePerspective(true)
			->setClipDistance(0.2f, 500.0f)
		)
		->setViewport(0, 0, iWidth, iHeight);


	m_scene = new SceneGraph();

	// initalize managers. also set context data in case it uses opengl function
	shaderDataMgr = new ResourceManager<ShaderInput>(loadBasicShaderData);
	materialMgr = new ResourceManager<Material>(loadBasicMaterial);
	meshMgr = new ResourceManager<Mesh>(loadMesh, "meshes", nullptr, (void*)ctxData);
	largeMeshMgr = new ResourceManager<LargeMesh>(loadLargeMesh, "meshes", nullptr, (void*)ctxData);
	textureMgr = new ResourceManager<Texture2D>(loadTexture, "textures", nullptr, (void*)ctxData);
	sourceMgr = new ResourceManager<ShaderSource>(loadShaderSource, "shaders", new ShaderSource(""), (void*)ctxData);

	// load a source
	sourceMgr->load("plain.glsl");

	// add a cube manually
	meshMgr->put("cube", Mesh::createUnitBox()->createBufferObjects());
	meshMgr->load("weirdcube.bcf");
	meshMgr->load("weirdsphere.bcf");
	meshMgr->load("sphere.bcf");
	
	assert(glGetError() == GL_NO_ERROR);
	//// load textures
	textureMgr->load("road_on_grass.png"); // ->withWrap(GL_REPEAT, GL_REPEAT);
	assert(glGetError() == GL_NO_ERROR);
	textureMgr->load("trimsheet_01.png");
	assert(glGetError() == GL_NO_ERROR);

	//textureMgr->get("env.jpg");
	//textureMgr->load("env.jpg");
	//textureMgr->load("env2.jpg");
	//textureMgr->load("env3.jpg");
	/*textureMgr->load("road_on_grass.png")->withWrap(GL_REPEAT, GL_REPEAT);
	textureMgr->load("trimsheet_01.png");*/
	//textureMgr->load("ibl_spec.jpg");
	//textureMgr->load("ibl_diff.jpg");
	// test making a simple effect
	StaticMeshObject* obj;
	ShaderFX* simple;
	MaterialSetting	setting;
	SceneNode* node = nullptr;

	simple = new ShaderFX();
	setting.setOpacityType(OpacityType::OPAQUE);
	simple->setTechnique(PipelineId::P_DRAW_UNLIT,
		(new ShaderTechnique)->addPass(
			(new RenderPass(
				sourceMgr->get("plain.glsl"),
				(new MaterialInput)->addTexture(textureMgr->get("trimsheet_01.png"))
			))->setMaterialSetting(setting)
		)
	);

	obj = new StaticMeshObject();
	obj->setMesh(meshMgr->get("weirdcube.bcf"))
		->setPosition(glm::vec3(-0.f,1.f,0.f))
		->setRotation(glm::angleAxis(glm::radians(30.f), glm::vec3(0, 1, 0)))
		->addEffect(simple);

	// add to scene graph?
	node = m_scene->addObject(obj);

	// another, different effect?
	// make a simple static mesh, with alpha cutout though
	simple = new ShaderFX();
	setting.setOpacityType(OpacityType::ALPHA_CLIP);
	simple->setTechnique(PipelineId::P_DRAW_UNLIT,
		(new ShaderTechnique)->addPass(
			(new RenderPass(
				sourceMgr->get("plain.glsl"),
				(new MaterialInput)->addTexture(textureMgr->get("trimsheet_01.png"))
			))->setMaterialSetting(setting)
		)
	);

	obj = new StaticMeshObject();
	obj->setMesh(meshMgr->get("weirdcube.bcf"))
		->setPosition(glm::vec3(-0.f, -0.f, 2.f))
		->setRotation(glm::angleAxis(glm::radians(30.f), glm::vec3(1, 0, 0)))
		->addEffect(simple);
	m_scene->addObject(obj, node);

	// debug print
	m_scene->printDebug();

	fbo = new FBO(iWidth, iHeight);
	rbo = new RBO(iWidth, iHeight);

	fbo->create();
	assert(glGetError() == GL_NO_ERROR);

	// must be bind because we're calling glFrameBufferRenderBuffer
	fbo->bind();
	rbo->create();
	assert(glGetError() == GL_NO_ERROR);

	assert(fbo->isComplete() && "FBO INCOMPLETE!");

	// clear em
	FBO::unbind();
	RBO::unbind();

	assert(glGetError() == GL_NO_ERROR);
}

void Game::onDestroy() {
	// debug first
	SDL_Log("++SHADERS_DATA++\n");
	shaderDataMgr->printDebug();
	SDL_Log("++MATERIALS++\n");
	materialMgr->printDebug();
	SDL_Log("++MESHES++\n");
	meshMgr->printDebug();
	SDL_Log("++LARGE_MESHES++\n");
	largeMeshMgr->printDebug();
	SDL_Log("++TEXTURES++\n");
	textureMgr->printDebug();
	SDL_Log("++SHADER_SOURCES++\n");
	sourceMgr->printDebug();
	
	destroyImGui();
	// delete renderer
	delete m_renderer;
	delete m_scene;
	// delete resource managers
	delete shaderDataMgr;
	delete meshMgr;
	delete largeMeshMgr;
	delete materialMgr;
	delete textureMgr;
	delete sourceMgr;

	// buffers
	delete fbo;
	delete rbo;
}

void Game::onUpdate(float dt) {
	// update renderer time
	m_renderer->updateTime(dt);
	// update scene graph
	m_scene->update(dt);

	textureMgr->get("env2.jpg");
	textureMgr->get("env.jpg");
}

void Game::onRender(float dt) {
	
	// compute camera
	cam_vertRot = glm::clamp(cam_vertRot, -89.0f, 89.0f);
	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, cam_dist);

	camPos = glm::rotate(camPos, glm::radians(cam_vertRot), glm::vec3(1, 0, 0));
	camPos = glm::rotate(camPos, glm::radians(cam_horzRot), glm::vec3(0, 1, 0));

	//SDL_Log("Camera pos: %.4f, %.4f, %.4f\n", camPos.x, camPos.y, camPos.z);
	m_renderer->getCamera()->setPosition(camPos);
	m_renderer->getCamera()->setTarget(glm::vec3(0, 0, 0));

	// set fbo
	fbo->bind();
	m_renderer->setViewport(0, 0, fbo->getWidth(), fbo->getHeight());
	// enable depth test
	glEnable(GL_DEPTH_TEST);

	// clear depth and color
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 3d renderer	
	m_renderer->draw(m_scene, dt);

	// debug?
	m_scene->static_tree->debugDraw(m_renderer);

	// unset fbo
	FBO::unbind();

	// 2d rendering
	glDisable(GL_DEPTH_TEST);

	beginRenderImGui();

	ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	{
		static int value = 20;
		static glm::vec4 color(0.5f, 0.2f, 1.0f, 1.0f);
		glClearColor(color.r, color.g, color.b, color.a);

		auto& io = ImGui::GetIO();
		std::string title;
		if (io.WantCaptureMouse) {
			title += "MOUSE_NEEDED";
		} else {
			title += "MOUSE_CLEAR";
		}

		// window app config
		ImGui::Begin("App Config", NULL, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %d", fps);
			ImGui::SameLine();
			ImGui::Text("%s (%.2f, %.2f)", title.c_str(), io.MousePos.x, io.MousePos.y);
			ImGui::SameLine();
			if (ImGui::Button("QUIT")) {
			    this->setRunFlag(false);
			}

			// statistic of pipeline
			const Pipeline* p = m_renderer->getPipeline(PipelineId::P_DRAW_UNLIT);
			if (p)
				p->drawDebugImGui();

			// some texture selector?
			static const char* currentTexture = NULL;
			static std::string selected;

			std::vector<std::string> textureNames;
			auto& resMap = textureMgr->getResourceMap();
			auto it = resMap.begin();
			while (it != resMap.end()) {
				textureNames.push_back(it->first);
				++it;
			}

			if (ImGui::BeginCombo("Texture List", currentTexture)) {
				for (int i = 0; i < (int)textureNames.size(); i++) {
					bool is_selected = selected == textureNames[i];

					if (ImGui::Selectable(textureNames[i].c_str(), is_selected)) {
						selected = textureNames[i];
						currentTexture = selected.c_str();
					}
					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}

			if (currentTexture) {
				Texture2D* tex = textureMgr->get(currentTexture);
				if (tex) {
					ImGui::Text("Texture ID: %u", tex->texId);
					ImGui::Image((ImTextureID)tex->texId, ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
				}
			}
			

		ImGui::End();

		ImGui::ShowMetricsWindow();

		ImGui::Begin("Platform Info", 0, ImGuiWindowFlags_AlwaysAutoResize);
			ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
			ImGui::Text("Monitor: %d", pio.Monitors.Size);
			ImGui::Text("Viewport count: %d", pio.Viewports.Size);
			ImGui::Text("Create Window: %x", pio.Platform_CreateWindow);
			ImGui::Text("Destroy Window: %x", pio.Platform_DestroyWindow);
			ImGui::Text("Render Window: %x", pio.Platform_RenderWindow);
		ImGui::End();


		// Main viewport (render/fbo)
		ImGui::SetNextWindowSize(ImVec2(iWidth, iHeight), ImGuiCond_FirstUseEver);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		char windowTitle[128];
		sprintf(windowTitle, "Main Window (%s)###MAIN_VIEWPORT", mainViewportFocused ? "ACTIVE" : "INACTIVE");
		ImGui::Begin(windowTitle, 0, ImGuiWindowFlags_NoScrollbar);
		ImGui::PopStyleVar(1);

		// grab available region size and current cursor pos (top left)
		ImVec2 regionSize = ImGui::GetContentRegionAvail();
		ImVec2 cursorPos = ImGui::GetCursorPos();

		// for cursor handling
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImVec2 margin = ImVec2(4.f, 4.f);
		ImVec2 minRect(windowPos.x + cursorPos.x, windowPos.y + cursorPos.y);
		ImVec2 maxRect(minRect.x + regionSize.x, minRect.y + regionSize.y);

		// track viewport focus status (only in focus if it's focused and cursor is within it)
		bool inRegion = io.MousePos.x > minRect.x + margin.x && io.MousePos.x < maxRect.x - margin.x
			&& io.MousePos.y > minRect.y + margin.y && io.MousePos.y < maxRect.y - margin.y;
		mainViewportFocused = ImGui::IsWindowFocused() && inRegion;
		
		// compute real usable pos and size for image widget
		ImVec2 imgPos;
		ImVec2 imgSize;
		float fboAspect = fbo->getAspect();

		Helper::computePosAndSize(regionSize, fboAspect, imgPos, imgSize);
		// offset it with cursor pos
		imgPos.x += cursorPos.x;
		imgPos.y += cursorPos.y;
		
		// draw it
		ImGui::SetCursorPos(imgPos);
		ImGui::Image((ImTextureID)fbo->getTextureId(), imgSize, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
	}

	endRenderImGui();
	
	// swap buffer?
	//SDL_GL_SwapWindow(wndApp);
}

void Game::onEvent(SDL_Event* e) {
	static int last_x=0, last_y=0;
	static bool dragging = false;
	static bool is_multigesturing = false;
	static int fingercount = 0;
	
	int cur_x, cur_y;

    auto& io = ImGui::GetIO();
    int* viewport = m_renderer->getViewport();

	bool canHandleInput = true;
	if (!ImGui_ImplBowie_ProcessEvent(e)) {
		canHandleInput = !ImGui_ImplSDL2_ProcessEvent(e);
	}
	ImGui_ImplSDL2_ProcessEvent(e);

	// in case of window resize, force inactivation?
	/*if (e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_RESIZED) {
		mainViewportFocused = false;
	}*/

	// let's print out event
	bool interesting = false;
	std::string eventName;
	switch (e->type) {
	    case SDL_FINGERDOWN:
	        interesting = true;
	        eventName = "FINGER_DOWN";
	        break;
	    case SDL_FINGERMOTION:
	        interesting = true;
	        eventName = "FINGER_MOTION";
	        break;
	    case SDL_FINGERUP:
	        interesting = true;
	        eventName = "FINGER_UP";
	        break;
	    case SDL_MULTIGESTURE:
	        interesting = true;
	        eventName = "MULTI_GESTURE";
	        break;
	    case SDL_MOUSEBUTTONDOWN:
	        interesting = true;
	        eventName = "MOUSE_DOWN";
	        break;
	    case SDL_MOUSEMOTION:
	        interesting = true;
	        eventName = "MOUSE_MOVE";
	        break;
	    case SDL_MOUSEBUTTONUP:
	        interesting = true;
	        eventName = "MOUSE_UP";
	        break;
	}


	switch (e->type) {
    case SDL_MULTIGESTURE:
        if (!mainViewportFocused && io.WantCaptureMouse) break;
        is_multigesturing = true;
        fingercount = 2;
        dragging = false;
        // now just update scale?
        cam_dist -= e->mgesture.dDist * 10.0f;
        break;
	case SDL_QUIT:
		this->setRunFlag(false);
		return;
	case SDL_WINDOWEVENT:
		if (e->window.event == SDL_WINDOWEVENT_RESIZED) {
			SDL_Log("Window resize: %d x %d\n", e->window.data1, e->window.data2);
			m_renderer->setViewport(0, 0, e->window.data1, e->window.data2);
		}
		break;
	// mimic dragging for finger down
	case SDL_FINGERDOWN:
		if (!mainViewportFocused && io.WantCaptureMouse || is_multigesturing) break;
		++fingercount;
		if (fingercount >= 2) {
		    is_multigesturing = true;
		    break;
		}
		dragging = true;
		cur_x = last_x = e->tfinger.x * viewport[2];
		cur_y = last_y = e->tfinger.y * viewport[3];
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (!mainViewportFocused && io.WantCaptureMouse) break;
		dragging = true;
		cur_x = last_x = e->button.x;
		cur_y = last_y = e->button.y;
		break;

	//// mimic mouse motion
	case SDL_FINGERMOTION:
		if (!mainViewportFocused && io.WantCaptureMouse || is_multigesturing) {
			break;
		}

		cur_x = e->tfinger.x * viewport[2];
		cur_y = e->tfinger.y * viewport[3];
		if (dragging) {
			cam_horzRot -= (float)(cur_x - last_x) * 0.5f;
			cam_vertRot -= (float)(cur_y - last_y) * 0.25f;
		}
		last_x = cur_x;
		last_y = cur_y;
		break;
	case SDL_MOUSEMOTION:
		if (!mainViewportFocused && io.WantCaptureMouse) break;
		cur_x = e->motion.x;
		cur_y = e->motion.y;
		if (dragging) {
			cam_horzRot -= (float)(cur_x - last_x);
			cam_vertRot -= (float)(cur_y - last_y);
		}
		last_x = cur_x;
		last_y = cur_y;
		break;
	case SDL_MOUSEWHEEL:
		if (!mainViewportFocused && io.WantCaptureMouse) break;
		cam_dist -= e->wheel.y * 0.2f;
		break;

	// mimic button up
	case SDL_FINGERUP:
	    --fingercount;
	    if (fingercount <0 ) fingercount = 0;
	    dragging = is_multigesturing = false;
	    break;
	case SDL_MOUSEBUTTONUP:
		if (!mainViewportFocused && io.WantCaptureMouse) break;
		dragging = false;
		break;

	}
}

void Game::initImGui() {
	// set proper scale
	float ddpi, hdpi, vdpi, scale = 1.0f;
	if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "No DPI Info available!");
	}
	else {
		SDL_Log("DPI Info (d, h, v): %.2f, %.2f, %.2f", ddpi, hdpi, vdpi);
		scale = (float)((int) (vdpi / 96.0f));
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// enable docking
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// drag from title bar only
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	// enable viewport?
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImFontConfig cfg;

	cfg.SizePixels = 13.0f * scale;
	io.Fonts->AddFontDefault(&cfg);

	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(scale);

	ImGui_ImplSDL2_InitForOpenGL(wndApp, glCtx);
	ImGui_ImplBowie_Init(wndApp);
}

void Game::destroyImGui() {
	ImGui_ImplBowie_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void Game::beginRenderImGui() {
	ImGui_ImplSDL2_NewFrame(wndApp);
	ImGui_ImplBowie_NewFrame();
	//ImGui_ImplBowie_InjectTouchHandler();
	ImGui::NewFrame();
}

void Game::endRenderImGui() {
	ImGui::Render();	// generate render data, before rendering for real

	// render it
	ImDrawData* data = ImGui::GetDrawData();
	ImGui_ImplBowie_RenderDrawData(data);

	auto& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		// save current window and context
		SDL_Window* currWindow = SDL_GL_GetCurrentWindow();
		SDL_GLContext currContext = SDL_GL_GetCurrentContext();
		// render other window
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		// restore window and context
		SDL_GL_MakeCurrent(currWindow, currContext);
	}
}