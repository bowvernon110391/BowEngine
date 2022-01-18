# source for imgui
file(GLOB_RECURSE IMGUI_SRC ./*.cpp)

add_library(imgui 
    STATIC
        ${IMGUI_SRC}
    )
