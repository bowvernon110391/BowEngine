#ifndef __APP_H__
#define __APP_H__

#include <glad/glad.h>
#include <SDL.h>
#include <SDL_platform.h>
#include <string>
#include "../imgui/imgui.h"

using std::string;

class App
{
private:
    /* data */
    bool bRun, bUseGLES;  // is it running? 

    void pollEvent();
protected:
    void setRunFlag(bool running) { bRun = running; }
    void setTickRate(int rate) { iTickRate = rate < 10 ? 10 : rate > 240 ? 240 : rate; }

    SDL_DisplayOrientation getScreenOrientation() {
        return SDL_GetDisplayOrientation(SDL_GetWindowDisplayIndex(wndApp));
    }

    void createWindow();

    SDL_Window *wndApp;
    int iWidth, iHeight;
    string szTitle;
    int iTickRate;

    int fps;

    SDL_GLContext glCtx;

    SDL_Renderer *renderer;
public:
    App(int tickRate = 50, const char* title="SDL App", int w = 800, int h = 600);
    ~App();

    // abstract function that must be implemented
    virtual void onUpdate(float dt) = 0;
    virtual void onRender(float dt) = 0;
    virtual void onInit() = 0;
    virtual void onDestroy() = 0;
    virtual void onEvent(SDL_Event *e) = 0;

    // implemented function
    void run();
};

#endif