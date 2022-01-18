
# define preprocessor (WEIRDLY only for MSVC, because otherwise it bugs out)
if (MSVC)
    add_compile_definitions(BT_USE_SSE_IN_API)
endif(MSVC)

include_directories(sdlgame ./bullet)

add_library(bullet 
    STATIC
        bullet/btBulletCollisionAll.cpp
        bullet/btBulletDynamicsAll.cpp
        bullet/btLinearMathAll.cpp
    )

include_directories(sdlgame ./glad/include)

add_library(glad
    STATIC
        glad/src/glad.c
    )

add_subdirectory(imgui)
add_subdirectory(engine)
add_subdirectory(game)  

