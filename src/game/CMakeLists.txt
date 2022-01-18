# now for the game executable

file(GLOB_RECURSE GAME_SRC ./*.cpp)

include_directories(sdlgame ../engine)

add_executable(game
    ${GAME_SRC}
    )

target_link_libraries(game 
    glad 
    bullet 
    imgui 
    engine
    ${SDL2_LIBRARIES}
    ${CMAKE_DL_LIBS}
    )

add_custom_command(TARGET game
    POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:game> ${PROJECT_SOURCE_DIR}/bin/)