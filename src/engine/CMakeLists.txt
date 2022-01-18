# engine source files
file(GLOB_RECURSE SRC ./*.cpp)

include_directories(sdlgame ${CMAKE_CURRENT_SOURCE_DIR})

add_library(engine
    STATIC
        ${SRC}
    )
