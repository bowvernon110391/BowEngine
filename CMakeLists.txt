cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(sdlgame LANGUAGES C CXX VERSION 0.1.0)

# find sdl2
find_package(SDL2 REQUIRED)
include_directories(sdlgame ${SDL2_INCLUDE_DIRS})

# find glm
find_package(glm REQUIRED)
include_directories(sdlgame ${GLM_INCLUDE_DIRS})

# include(CTest)
# enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(src)
