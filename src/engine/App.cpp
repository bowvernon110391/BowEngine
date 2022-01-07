#include "App.h"

App::App(int tickRate, const char* title, int w, int h):
bRun(true), iWidth(w), iHeight(h), szTitle(title), fps(0) {
    SDL_Init(
        SDL_INIT_VIDEO 
        | SDL_INIT_GAMECONTROLLER 
        | SDL_INIT_TIMER 
        | SDL_INIT_JOYSTICK
    );
    
    setTickRate(tickRate);
    createWindow();
}

App::~App() {
}

void App::createWindow() {
    // auto adjust screen size for android
    Uint32 windowFlags = SDL_WINDOW_RESIZABLE;  // for desktop
    const string platformName = string(SDL_GetPlatform());

    SDL_Log("Detected platform: %s", platformName.c_str());
    bUseGLES = false;

    if (platformName == string("Android") || platformName == string("iOS")) {

        // disable touch mouse fake event on mobile
        SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
        SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");

        bUseGLES = true;

        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
            // no error
            iWidth = dm.w;
            iHeight = dm.h;

            windowFlags = SDL_WINDOW_FULLSCREEN;
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Failed setting auto screen resolution: %s", SDL_GetError());
        }
        
    }

    SDL_Log("Creating window: %s, %d x %d", szTitle.c_str(), iWidth, iHeight);

    // init opengl before window creation
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    if (bUseGLES) {
        SDL_Log("Forcing GLES Context for embedded thing...");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }
    else {
        SDL_Log("Forcing core context OpenGL 2.1");
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    }
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // ANTI ALIASING
     SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
     SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // set hint
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeRight");

    // create the window
    wndApp = SDL_CreateWindow(szTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        iWidth, iHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | windowFlags);

    if (!wndApp) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Window creation failed: %s", SDL_GetError());
    }


    glCtx = SDL_GL_CreateContext(wndApp);

    // initialize glad
    bool errGlad = bUseGLES ? (gladLoadGLES2Loader(SDL_GL_GetProcAddress) == 0) : (gladLoadGLLoader(SDL_GL_GetProcAddress) == 0);
    if (errGlad) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "failed to initalize GLAD context!");
        exit(-1);
    }
    SDL_Log("GLAD context initialized!");

    // create renderer that is hardware accelerated and support render to texture
    renderer = SDL_CreateRenderer(wndApp, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Renderer creation failed: %s", SDL_GetError());
    }
    else {
        // vsync stuffs?
        if (SDL_GL_SetSwapInterval(1) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to enable vsync: %s", SDL_GetError());
        }
        else {
            SDL_Log("Vsync enabled.");
        }
    }
}

void App::pollEvent() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        onEvent(&e);
    }
}

// The run
void App::run() {
    // runtime data
    Uint32 prevTick, currTick, delta, fpsTimer = 0;
    prevTick = SDL_GetTicks();
    int fpsCounter = 0;

    float dt = 1.0f / (float)iTickRate;
    float accumDt = 0;

    // call init first
    SDL_Log("Initializing app...\n");
    onInit();

    SDL_Log("Entering main loop...\n");

    while (bRun) {
        // grab our new tick
        // and accumulate timer
        currTick = SDL_GetTicks();
        delta = (currTick - prevTick);
        accumDt += delta * 0.001f;
        prevTick = currTick;

        // update in fixed timesteps
        while (accumDt >= dt) {
            // poll event
            pollEvent();
            // do update here
            onUpdate(dt);
            // remove accumulated timer
            accumDt -= dt;
        }
        // render with remaining time
        onRender(accumDt);
        SDL_GL_SwapWindow(wndApp);

        fpsCounter++;
        fpsTimer += delta;
        if (fpsTimer >= 1000) {
            // reset timer?
            fpsTimer = 0;
            fps = fpsCounter;
            fpsCounter = 0;
        }
    }

    // destroy last
    SDL_Log("Destroying app...\n");
    onDestroy();

    // clean up
	SDL_Log("Destroying renderer...");
    SDL_DestroyRenderer(renderer);

	SDL_Log("Deleting OpenGL Context...");
    SDL_GL_DeleteContext(glCtx);
	
	SDL_Log("Destroying window...");
    SDL_DestroyWindow(wndApp);

    // quit
	SDL_Log("Quitting SDL...");
    SDL_Quit();
}
