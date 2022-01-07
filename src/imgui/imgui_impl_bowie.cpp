#include "imgui_impl_bowie.h"
#include "../engine/Texture2d.h"
#include "../engine/Shader.h"
#include "../engine/MaterialInputs.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//static Texture2D* fontTexture = 0;
//static Shader* fontShader = 0;
//static ShaderInput* fontShaderInput = 0;
//static glm::mat4 projMat;
//static GLuint vboHandle;
//static GLuint iboHandle;
//static bool showVirtualKeyboard = false;
//static SDL_Window* wndApp = 0;
//static TouchData touchData;
//static bool useTouchScreen = false;

//Texture2D* getFontTexture() {
//	return fontTexture;
//}

//TouchData* ImGui_ImplBowie_GetTouchData() {
//	return &touchData;
//}


typedef struct _TouchData {
	_TouchData() {
		cX = cY = 0;
		buttonDown[0] = false;
		buttonDown[1] = false;
		buttonDown[2] = false;
	}
	int cX, cY;
	bool buttonDown[3];
} TouchData;

typedef struct _BowieData {
	Shader* fontShader;
	MaterialInput* fontShaderInput;
	Texture2D* fontTexture;
	GLuint vboHandle;
	GLuint iboHandle;
	SDL_Window* window;

	_BowieData() {
		fontShader = NULL;
		fontTexture = NULL;
		fontShaderInput = NULL;

		vboHandle = iboHandle = 0;
	}

	~_BowieData() {
		if (fontShader)
			delete fontShader;
		if (fontTexture)
			delete fontTexture;
		if (fontShaderInput)
			delete fontShaderInput;
	}
} BowieData;

static BowieData* ImGui_ImplBowie_GetBackendData() {
	return ImGui::GetCurrentContext() ? (BowieData*)ImGui::GetIO().BackendRendererUserData : NULL;
}

static void ImGui_ImplBowie_InitPlatformInterface();
static void ImGui_ImplBowie_ShutdownPlatformInterface();

bool ImGui_ImplBowie_Init(SDL_Window *wnd) {
	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.BackendRendererUserData == NULL && "Already initialized a renderer backend!");

	SDL_Log("Initializing bowie's IMGUI Implementation...");

	BowieData* bd = IM_NEW(BowieData)();

	bd->window = wnd;

	io.BackendRendererUserData = (void*)bd;
	io.BackendRendererName = "imgui_impl_bowie_sdl";

	// activate viewports?
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

	// activate viewport platforms?
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		ImGui_ImplBowie_InitPlatformInterface();

	return true;

	//std::string platform = SDL_GetPlatform();
	//if (platform == "Android" || platform == "iOS") {
	//	useTouchScreen = true;
	//}

	//wndApp = wnd;
	//io.BackendRendererName = "imgui_impl_bowie";

	//io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

	//// create device objects here!
	//SDL_Log("Creating device objects...");
	//ImGui_ImplBowie_CreateDeviceObjects();

	//// init rendering for multi viewport
	//IM_ASSERT((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) && "Multi Viewports must be enabled!");
	//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
	//	ImGui_ImplBowie_InitPlatformInterface();
	//}

	//return (NULL != fontTexture) && (NULL != fontShader);
}

void ImGui_ImplBowie_Shutdown() {
	BowieData* bd = ImGui_ImplBowie_GetBackendData();
	IM_ASSERT(bd != NULL && "No renderer backend to shutdown, probably already shut");

	ImGuiIO& io = ImGui::GetIO();

	ImGui_ImplBowie_ShutdownPlatformInterface();
	ImGui_ImplBowie_DestroyDeviceObjects();
	io.BackendRendererName = NULL;
	io.BackendRendererUserData = NULL;
	IM_DELETE(bd);
}

//void ImGui_ImplBowie_InjectTouchHandler() {
//	if (useTouchScreen) {
//		auto& io = ImGui::GetIO();
//
//		io.MousePos = ImVec2(touchData.cX, touchData.cY);
//		io.MouseDown[0] = touchData.buttonDown[0];
//	}
//}

void ImGui_ImplBowie_NewFrame() {
	static bool lastKeyboardShown = false;

	// for now, do nothing
	//SDL_Log("Imgui_Bowie_NewFrame()");
	// show virtual keyboard when needed
	ImGuiIO& io = ImGui::GetIO();

	// track the keyboard shown flag
	BowieData* bd = (BowieData*)io.BackendRendererUserData;
	IM_ASSERT(bd && "Backend data is NULL!!");
	IM_ASSERT(bd->window && "No SDL Window set for backend!");

	// create device objects if it hasn't
	if (!bd->fontShader) {
		ImGui_ImplBowie_CreateDeviceObjects();
	}

	bool keyboardShown = SDL_IsScreenKeyboardShown(bd->window);

	// gotta check some shiet
	if (io.WantTextInput && !keyboardShown) {
		// text input wanted, keyboard not shown, show it
		SDL_StartTextInput();
	}

	// maybe no longer needed
	// or still in text mode when not needed
	if (!io.WantTextInput && (keyboardShown || SDL_IsTextInputActive())) {
		SDL_StopTextInput();
	}

	// stop text input if now keyboard is hidden but last time it was shown
	if (!keyboardShown && lastKeyboardShown) {
		ImGui::SetWindowFocus(nullptr);
	}

	// track last keyboard shown value
	lastKeyboardShown = keyboardShown;

	// show log status?
	static int backspaceCounter = 0;
	static int returnCounter = 0;
	bool specialHandleMode = io.WantTextInput && SDL_IsScreenKeyboardShown(bd->window);

	if (specialHandleMode) {
		//SDL_Log("BS: %d", io.KeysDown[SDL_SCANCODE_BACKSPACE]);
		// gotta check if we're doing more than one frame already
		if (io.KeysDown[SDL_SCANCODE_BACKSPACE]) {
			////SDL_Log("BACKSPACE IS ON WITH COUNTER %d", backspaceCounter);
			if (++backspaceCounter > 1) {
				//SDL_Log("BACKSPACE OFF @ %d", backspaceCounter);
				backspaceCounter = 0;
				io.KeysDown[SDL_SCANCODE_BACKSPACE] = false;
			}
		}

		// handle return key separately
		if (io.KeysDown[SDL_SCANCODE_RETURN]) {
			if (++returnCounter > 1) {
				returnCounter = 0;
				io.KeysDown[SDL_SCANCODE_RETURN] = false;
			}
		}
	}
}

bool ImGui_ImplBowie_CreateFontsTexture() {
	BowieData* bd = ImGui_ImplBowie_GetBackendData();
	IM_ASSERT(bd && "No backend data");

	if (!bd->fontTexture) {
		// grab texture data from imgui
		ImGuiIO& io = ImGui::GetIO();

		unsigned char* pixels;
		int w, h;

		io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);

		bd->fontTexture = new Texture2D();
		bd->fontTexture->width = w;
		bd->fontTexture->height = h;
		bd->fontTexture->texData = pixels;
		bd->fontTexture->useMipmap = false;
		bd->fontTexture->minFilter = GL_NEAREST;

		if (!bd->fontTexture->upload()) {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed creating font texture!");
			delete bd->fontTexture;
			bd->fontTexture = NULL;
			return false;
		}
		else {
			SDL_Log("Font texture created.");
			bd->fontTexture->texData = NULL;
			// set font texture data
			io.Fonts->SetTexID((ImTextureID)(intptr_t)bd->fontTexture->texId);
			bd->fontShaderInput = (new MaterialInput())->addTexture(bd->fontTexture);

			return true;
		}
	}
	return true;
}

// create font shader for our implementation
static void ImGui_ImplBowie_CreateFontShader() {
	BowieData* bd = ImGui_ImplBowie_GetBackendData();
	IM_ASSERT(bd && "No backend data");

	const char* vertShader = R"(
uniform mat4 m_projection;

attribute vec2 position;
attribute vec2 uv;
attribute vec4 color;

varying vec4 vColor;
varying vec2 vTexcoord;

void main() {
    vColor = color;
    vTexcoord = uv;
    gl_Position = m_projection * vec4(position.xy, 0, 1);
}
    )";

	const char* fragShader =
	        "#ifdef GL_ES\n"
            "precision mediump float;\n"
            "#endif\n"
            "uniform sampler2D texture0;\n"
            "varying vec4 vColor;\n"
            "varying vec2 vTexcoord;\n"
            "void main() {\n"
            "vec4 color = texture2D(texture0, vTexcoord);\n"
            "gl_FragColor = vColor * color;\n"
            "}\n";

	SDL_Log("Font Shader: %s", fragShader);

	if (!bd->fontShader) {
		//fontShader = Shader::loadShaderFromSources(vertShader, strlen(vertShader), fragShader, strlen(fragShader));
		bd->fontShader = Shader::fromMemory(vertShader, strlen(vertShader), fragShader, strlen(fragShader));

		// setup some uniform
		if (bd->fontShader) {
			SDL_Log("Font shader loaded");
			bd->fontShader->printDebug();
		}
		else {
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed loading font shader!");
		}
	}
}

void ImGui_ImplBowie_CreateDeviceObjects() {
	BowieData* bd = ImGui_ImplBowie_GetBackendData();
	IM_ASSERT(bd && "No backend data");

	// just create texture and shader
	ImGui_ImplBowie_CreateFontsTexture();
	ImGui_ImplBowie_CreateFontShader();

	// init vbo and ibo
	glGenBuffers(1, &bd->vboHandle);
	glGenBuffers(1, &bd->iboHandle);
}

// cleanup functions
void ImGui_ImplBowie_DestroyDeviceObjects() {
	BowieData* bd = ImGui_ImplBowie_GetBackendData();
	IM_ASSERT(bd && "No backend data");

	if (bd->vboHandle) {
		glDeleteBuffers(1, &bd->vboHandle); 
		bd->vboHandle = 0;
	}

	if (bd->iboHandle) {
		glDeleteBuffers(1, &bd->iboHandle);
		bd->iboHandle = 0;
	}
}


// drawing function!
// prepare rendering
static void Imgui_ImplBowie_SetupRenderState(int fb_width, int fb_height, ImDrawData* draw_data) {
	BowieData* bd = ImGui_ImplBowie_GetBackendData();

	IM_ASSERT(bd && bd->fontShader && "NO FONT SHADER LOADED!");
	//IM_ASSERT(bd->fontTexture && "NO FONT TEXTURE LOADED!");

	// setup gl state
	glEnable(GL_BLEND);
	glBlendEquation(GL_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, fb_width, fb_height);

	// setup orthogonal matrix
	float L = draw_data->DisplayPos.x;
	float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
	float T = draw_data->DisplayPos.y;
	float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;

	const float ortho_projection[4][4] =
	{
		{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
		{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
		{ 0.0f,         0.0f,        -1.0f,   0.0f },
		{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
	};

	//glm::mat4 projMat = glm::ortho((float)0, (float)(R - L), (float)(B - T), (float)0 , -1.0f, 1.0f);

	// setup shader
	bd->fontShader->bind();
	//bd->fontShaderInput->setupData(bd->fontShader);
	// projection matrix
	//glUniformMatrix4fv(bd->fontShader->getUniformLocation(Shader::UniformLoc::m_projection), 1, false, glm::value_ptr(projMat));
	glUniformMatrix4fv(bd->fontShader->getUniformLocation(Shader::UniformLoc::m_projection), 1, false, &ortho_projection[0][0]);
	
	// setup buffers
	glBindBuffer(GL_ARRAY_BUFFER, bd->vboHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bd->iboHandle);

	int position_loc = bd->fontShader->getAttribLocation(Shader::AttribLoc::position);
	int color_loc = bd->fontShader->getAttribLocation(Shader::AttribLoc::color);
	int uv_loc = bd->fontShader->getAttribLocation(Shader::AttribLoc::uv);

	glEnableVertexAttribArray(position_loc);
	glEnableVertexAttribArray(color_loc);
	glEnableVertexAttribArray(uv_loc);

	glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
}

void ImGui_ImplBowie_RenderDrawData(ImDrawData* draw_data) {
	int fbWidth = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
	int fbHeight = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

	if (fbWidth <= 0 || fbHeight <= 0)
		return;

	BowieData* bd = ImGui_ImplBowie_GetBackendData();
	// for now, just log some shit perhaps?
	//SDL_Log("Imgui_Bowie_RenderDrawData(): w = %d, h = %d, list = %d", fbWidth, fbHeight, draw_data->CmdListsCount);

	// backup state
	GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
	glActiveTexture(GL_TEXTURE0);
	GLuint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);
	GLuint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
	GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
	GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
	GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
	GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
	GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	GLuint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
	GLuint last_element_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, (GLint*)&last_element_buffer);

	// setup render state now (change states)
	Imgui_ImplBowie_SetupRenderState(fbWidth, fbHeight, draw_data);

	ImVec2 clip_off = draw_data->DisplayPos;
	ImVec2 clip_scale = draw_data->FramebufferScale;

	// now render them all?
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[n];

		// buffer vertex data
		glBufferData(
			GL_ARRAY_BUFFER, 
			(GLsizeiptr)(cmd_list->VtxBuffer.Size * sizeof(ImDrawVert)), 
			(const GLvoid*)cmd_list->VtxBuffer.Data, 
			GL_STREAM_DRAW
		);
		// buffer index data
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER, 
			(GLsizeiptr)(cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx)), 
			(const GLvoid*)cmd_list->IdxBuffer.Data, 
			GL_STREAM_DRAW
		);

		// draw command
		for (int cmdIdx = 0; cmdIdx < cmd_list->CmdBuffer.Size; cmdIdx++) {
			const ImDrawCmd* pCmd = &cmd_list->CmdBuffer[cmdIdx];

			if (pCmd->UserCallback != NULL) {
				// draw command
				if (pCmd->UserCallback == ImDrawCallback_ResetRenderState) {
					Imgui_ImplBowie_SetupRenderState(fbWidth, fbHeight, draw_data);
				}
				else {
					// custom callback
					pCmd->UserCallback(cmd_list, pCmd);
				}
			}
			else {
				// draw data
				// do scissor test (might show incorrect though)
				ImVec4 clipRect = pCmd->ClipRect;

				ImVec2 clip_min((pCmd->ClipRect.x - clip_off.x) * clip_scale.x, (pCmd->ClipRect.y - clip_off.y) * clip_scale.y);
				ImVec2 clip_max((pCmd->ClipRect.z - clip_off.x) * clip_scale.x, (pCmd->ClipRect.w - clip_off.y) * clip_scale.y);
				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
					continue;

				// Apply scissor/clipping rectangle (Y is inverted in OpenGL)
				glScissor((int)clip_min.x, (int)(fbHeight - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y));


				//SDL_Log("Cliprect: %.2f %.2f %.2f %.2f", clipRect.x, clipRect.y, clipRect.z, clipRect.w);

				// only draw if within view
				// bind texture data
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pCmd->TextureId);
				// draw elements
				glDrawElements(
					GL_TRIANGLES,
					pCmd->ElemCount,
					sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
					(const GLvoid*)(pCmd->IdxOffset * sizeof(ImDrawIdx))
				);
			}
		}
	}

	// restore state
	glUseProgram(last_program);
	glBindTexture(GL_TEXTURE_2D, last_texture);
	glActiveTexture(last_active_texture);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_buffer);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
	if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if (last_enable_stencil_test) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
	if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);

	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);

}

bool ImGui_ImplBowie_ProcessEvent(SDL_Event* e) {
	BowieData* bd = ImGui_ImplBowie_GetBackendData();
	ImGuiIO& io = ImGui::GetIO();
	bool specialHandleMode = io.WantTextInput && SDL_IsScreenKeyboardShown(bd->window);

	float width = io.DisplaySize.x * io.DisplayFramebufferScale.x;
	float height = io.DisplaySize.y * io.DisplayFramebufferScale.y;

	// check backspace key status
	// now handle specific event here
	switch (e->type) {
	case SDL_KEYUP:
		// prevent imgui to handle backspace + enter release
		if (specialHandleMode) {
			if (e->key.keysym.scancode == SDL_SCANCODE_BACKSPACE 
				|| e->key.keysym.scancode == SDL_SCANCODE_RETURN) {
				return true;
			}
		}
		break;
	case SDL_KEYDOWN:
		// handle special text input for mobile devices
		if (specialHandleMode) {
			// okay, if it's return key, add character
			/*if (e->key.keysym.scancode == SDL_SCANCODE_RETURN) {
				io.AddInputCharacter('\n');
				return true;
			}*/

			// if it's backspace, do something else
			if (e->key.keysym.scancode == SDL_SCANCODE_BACKSPACE
				|| e->key.keysym.scancode == SDL_SCANCODE_RETURN) {
				//SDL_Log("BACKSPACE oldstate %d", io.KeysDown[SDL_SCANCODE_BACKSPACE]);
				//SDL_Log("Forcing BACKSPACE!");
				io.KeysDown[e->key.keysym.scancode] = true;
				return true;
			}
		}
		break;
	/*case SDL_FINGERDOWN:
		touchData.buttonDown[0] = true;
		touchData.cX = e->tfinger.x * width;
		touchData.cY = e->tfinger.y * height;
		break;
	case SDL_FINGERMOTION:
		touchData.cX = e->tfinger.x * width;
		touchData.cY = e->tfinger.y * height;
		break;
	case SDL_FINGERUP:
		touchData.buttonDown[0] = false;
		touchData.cX = e->tfinger.x * width;
		touchData.cY = e->tfinger.y * height;
		break;*/
	}
	
	return false;
}

static void ImGui_ImplBowie_RenderWindow(ImGuiViewport* v, void* d) {
	/*SDL_Log("Im drawing window[%u] @ %.2f, %.2f ; %.2f x %.2f", 
		v->ID, v->Pos.x, v->Pos.y,
		v->Size.x, v->Size.y
	);*/

	// if window specify render clear, clear it
	if (!(v->Flags & ImGuiViewportFlags_NoRendererClear)) {
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	ImGui_ImplBowie_RenderDrawData(v->DrawData);
}

static void ImGui_ImplBowie_InitPlatformInterface() {
	ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
	pio.Renderer_RenderWindow = ImGui_ImplBowie_RenderWindow;
}

static void ImGui_ImplBowie_ShutdownPlatformInterface() {
	ImGui::DestroyPlatformWindows();
}