# default compiler flags
SDL_CFL := $(shell pkg-config --cflags --libs sdl2)
GL_CFL := -lGL -lGLU -ldl
# SDL_CFL := $(filter-out -mwindows,$(SDL_CFL))
EXE_FILENAME := sdlgame
BIN_DIR := bin

CFLAGS := -I./src/glad/include

# when debug, disable optimization
# and output debug symbol
ifeq ($(BUILD),debug)
  CFLAGS += -O0 -g
else
  CFLAGS += -O3 -s -DNDEBUG
endif

# if non windows is used
ifeq ($(OS),Windows_NT)
  EXE_FILENAME := $(EXE_FILENAME).exe
  GL_CFL := -lopengl32 -lglu32
endif


all:
#	@echo $(SDL_CFL)
	@echo compiling on: $(OS), $(GL_CFL)
	g++ -m64 $(CFLAGS) src/*.cpp src/imgui/*.cpp src/glad/src/glad.c -o $(BIN_DIR)/$(EXE_FILENAME) $(SDL_CFL) $(GL_CFL)

debug:
	make "BUILD=debug"

clean:
	$(RM) $(BIN_DIR)/*.exe

run:
	@echo running executables...
	@cd bin && pwd && ./$(EXE_FILENAME)
