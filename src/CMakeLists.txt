cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(sdlgame LANGUAGES C CXX)

include_directories(sdlgame ./glad/include)
include_directories(sdlgame ./bullet)

# find sdl2
find_package(SDL2 REQUIRED)
include_directories(sdlgame ${SDL2_INCLUDE_DIRS})

# find glm
# find_package(GLM REQUIRED)
# include_directories(sdlgame ${GLM_INCLUDE_DIRS})

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
    AABBTree.cpp
    AbstractRenderObject.cpp
    App.cpp
    Camera.cpp
    Game.cpp
    Helper.cpp
    LargeMesh.cpp
    Loader.cpp
    main.cpp
    MatrixInput.cpp
    MaterialInputs.cpp
    Mesh.cpp
    Pipeline.cpp
    Renderable.cpp
    RenderCommand.cpp
    Renderer.cpp
    SceneData.cpp
    SceneGraph.cpp
    Shader.cpp
    ShaderFX.cpp
    ShaderInstanceManager.cpp
    ShaderSource.cpp
    StaticMeshObject.cpp
    Texture2d.cpp
    UnlitPipeline.cpp
    )

target_link_libraries(game 
    glad 
    bullet 
    imgui 
    ${SDL2_LIBRARIES}
    ${CMAKE_DL_LIBS}
    )