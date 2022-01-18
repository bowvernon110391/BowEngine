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

# define preprocessor (WEIRDLY only for MSVC, because otherwise it bugs out)
if (MSVC)
    add_compile_definitions(BT_USE_SSE_IN_API)
endif(MSVC)

# source for imgui
file(GLOB_RECURSE IMGUI_SRC imgui/*.cpp)

add_library(imgui 
    STATIC
        ${IMGUI_SRC}
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

# engine source files
file(GLOB_RECURSE SRC engine/*.cpp)

add_executable(game
    ${SRC}
    )

target_link_libraries(game 
    glad 
    bullet 
    imgui 
    ${SDL2_LIBRARIES}
    ${CMAKE_DL_LIBS}
    )

add_custom_command(TARGET game
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:game> ../bin/)   

