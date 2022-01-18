#ifndef __GAME_H__
#define __GAME_H__

#include "App.h"
#include "ResourceManager.h"
#include <vector>
class Renderer;
class AbstractRenderObject;
class Shader;
class Mesh;
class ShaderInput;
class Material;
class Texture2D;
class LargeMesh;
class AABBTree;
class AABBNode;
class ShaderSource;
class ShaderInstanceManager;
class SceneGraph;


// #define GLM_FORCE_MESSAGES
typedef struct _ContextData {
    _ContextData(SDL_Window* wnd, const SDL_GLContext ctx) {
        window = wnd;
        glContext = ctx;
    }

    SDL_Window* window;
    SDL_GLContext glContext;
} ContextData;

class Game : public App
{
private:
    
public:
    Game(/* args */);
    virtual ~Game();
    
    void onInit();
    void onDestroy();
    void onUpdate(float dt);
    void onRender(float dt);
    void onEvent(SDL_Event *e);

private:
    void initImGui();
    void destroyImGui();
    void beginRenderImGui();
    void endRenderImGui();

    // some of our loaders, not necessarily a file
    static ShaderInput* loadBasicShaderData(const char* name, void* pdata=0);
    static Mesh* loadMesh(const char* name, void* pdata=0);
    static Texture2D* loadTexture(const char* name, void* pdata=0);
    static Material* loadBasicMaterial(const char* name, void* pdata=0);
    static LargeMesh* loadLargeMesh(const char* name, void* pdata=0);
    static ShaderSource* loadShaderSource(const char* name, void* pdata=0);

    // THE renderer
    Renderer* m_renderer;

    // THE scenegraph
    SceneGraph* m_scene;

    // will replace with proper manager later...
    ResourceManager<Mesh> *meshMgr;
    ResourceManager<LargeMesh>* largeMeshMgr;
    ResourceManager<ShaderInput> *shaderDataMgr;
    ResourceManager<Material> *materialMgr;
    ResourceManager<Texture2D> *textureMgr;
    ResourceManager<ShaderSource>* sourceMgr;

    ContextData* ctxData;

    float cam_horzRot, cam_vertRot, cam_dist;
    ImVec4 clearColor;
};


#endif