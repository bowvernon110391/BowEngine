cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(sdlgame LANGUAGES C CXX)

include_directories(sdlgame ./glad/include)
include_directories(sdlgame ./bullet)

# find sdl2
find_package(SDL2 REQUIRED)
include_directories(sdlgame ${SDL2_INCLUDE_DIRS})

# find glm
find_package(glm REQUIRED)
include_directories(sdlgame ${GLM_INCLUDE_DIRS})

add_library(imgui 
    STATIC
        imgui/imgui_draw.cpp
        imgui/imgui_impl_bowie.cpp
        imgui/imgui_impl_sdl.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui.cpp
    )

add_library(bullet 
    STATIC
        bullet/btBulletCollisionAll.cpp
        bullet/btBulletDynamicsAll.cpp
        bullet/btLinearMathAll.cpp
    )

add_library(glad
    STATIC
        glad/src/glad.c
    )

add_executable(game
    engine/AABBTree.cpp
    engine/AbstractRenderObject.cpp
    engine/App.cpp
    engine/Camera.cpp
    engine/Game.cpp
    engine/Helper.cpp
    engine/LargeMesh.cpp
    engine/Loader.cpp
    engine/main.cpp
    engine/MatrixInput.cpp
    engine/MaterialInputs.cpp
    engine/Mesh.cpp
    engine/Pipeline.cpp
    engine/Renderable.cpp
    engine/RenderCommand.cpp
    engine/Renderer.cpp
    engine/SceneData.cpp
    engine/SceneGraph.cpp
    engine/Shader.cpp
    engine/ShaderFX.cpp
    engine/ShaderInstanceManager.cpp
    engine/ShaderSource.cpp
    engine/StaticMeshObject.cpp
    engine/Texture2d.cpp
    engine/UnlitPipeline.cpp
    engine/BufferObjects.cpp
    )

target_link_libraries(game 
    glad 
    bullet 
    imgui 
    ${SDL2_LIBRARIES}
    ${CMAKE_DL_LIBS}
    )