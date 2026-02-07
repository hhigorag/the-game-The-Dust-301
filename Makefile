# Compiladores
CC = gcc
CXX = g++

# Diretórios
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
EXTERNAL_DIR = external

# Caminho do raylib (ajuste se necessário)
RAYLIB_DIR = $(EXTERNAL_DIR)/raylib
RAYLIB_INCLUDE = $(RAYLIB_DIR)/include
RAYLIB_LIB = $(RAYLIB_DIR)/lib

# Caminho do ENet (compilado do zero)
ENET_PATH = $(EXTERNAL_DIR)/enet
ENET_BUILD = $(ENET_PATH)/build
ENET_INCLUDE = $(ENET_PATH)/include

# Flags de compilação C
# USE_RLGL: descomente para rlgl.h (raylib); comente se raylib mudar e rlgl não existir
# DEBUG: only in "make debug" (wireframe, etc.). Release = "make" without DEBUG.
CFLAGS = -Wall -Wextra -std=c99 -O2 -D_DEFAULT_SOURCE -DUSE_ENET -DUSE_RLGL

# Flags de compilação C++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 -D_DEFAULT_SOURCE

# Includes
INCLUDES = -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/core -I$(INCLUDE_DIR)/app -I$(RAYLIB_INCLUDE) -I$(ENET_INCLUDE)
LDFLAGS = -L$(RAYLIB_LIB) -L$(ENET_BUILD)

# Bibliotecas
LIBS = -lraylib -lenet -lws2_32 -lwinmm -lgdi32 -luser32

# Arquivos fonte Core
CORE_SRC = $(SRC_DIR)/core/core.c \
           $(SRC_DIR)/core/time.c \
           $(SRC_DIR)/core/state/match_state.c \
           $(SRC_DIR)/core/state/game_state.c \
           $(SRC_DIR)/core/state/lobby_state.c \
           $(SRC_DIR)/core/math/rng.c \
           $(SRC_DIR)/core/math/mat4.c \
           $(SRC_DIR)/core/world/map.c \
           $(SRC_DIR)/core/world/procgen.c \
           $(SRC_DIR)/core/world/world_seed.c \
           $(SRC_DIR)/core/world/chunk.c \
           $(SRC_DIR)/core/world/voxel_world.c \
           $(SRC_DIR)/core/world/route.c \
           $(SRC_DIR)/core/world/checkpoint.c \
           $(SRC_DIR)/core/world/zones.c \
           $(SRC_DIR)/core/world/structures.c \
           $(SRC_DIR)/core/world/world_generator.c \
           $(SRC_DIR)/core/gameplay/player.c \
           $(SRC_DIR)/core/physics/physics.c \
           $(SRC_DIR)/core/net/net.c

# Arquivos fonte App (C)
APP_SRC_C = $(SRC_DIR)/app/app.c \
            $(SRC_DIR)/app/settings/settings.c \
            $(SRC_DIR)/app/scenes/scene_manager.c \
            $(SRC_DIR)/app/scenes/scene_menu.c \
            $(SRC_DIR)/app/scenes/scene_gameplay.c \
            $(SRC_DIR)/app/input/input.c \
            $(SRC_DIR)/app/render/voxel_renderer.c \
            $(SRC_DIR)/app/render/voxel_mesh.c \
            $(SRC_DIR)/app/render/frustum.c \
            $(SRC_DIR)/app/render/atmosphere.c \
            $(SRC_DIR)/app/render/lighting.c \
            $(SRC_DIR)/app/ui/scifi_terminal.c

# Arquivos fonte App (C++)
APP_SRC_CPP = $(SRC_DIR)/app/camera/fps_camera.cpp

# Todos os arquivos fonte
ALL_SRC_C = $(SRC_DIR)/main.c $(CORE_SRC) $(APP_SRC_C)
ALL_SRC_CPP = $(APP_SRC_CPP)

# Arquivos objeto
OBJS_C = $(ALL_SRC_C:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJS_CPP = $(ALL_SRC_CPP:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJS = $(OBJS_C) $(OBJS_CPP)

# Nome do executável
TARGET = $(BUILD_DIR)/game.exe

# Regra padrão
all: $(TARGET)

# Cria diretórios de build
$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(BUILD_DIR)\core" mkdir "$(BUILD_DIR)\core"
	@if not exist "$(BUILD_DIR)\core\state" mkdir "$(BUILD_DIR)\core\state"
	@if not exist "$(BUILD_DIR)\core\math" mkdir "$(BUILD_DIR)\core\math"
	@if not exist "$(BUILD_DIR)\core\world" mkdir "$(BUILD_DIR)\core\world"
	@if not exist "$(BUILD_DIR)\core\math" mkdir "$(BUILD_DIR)\core\math"
	@if not exist "$(BUILD_DIR)\core\gameplay" mkdir "$(BUILD_DIR)\core\gameplay"
	@if not exist "$(BUILD_DIR)\core\physics" mkdir "$(BUILD_DIR)\core\physics"
	@if not exist "$(BUILD_DIR)\core\net" mkdir "$(BUILD_DIR)\core\net"
	@if not exist "$(BUILD_DIR)\app" mkdir "$(BUILD_DIR)\app"
	@if not exist "$(BUILD_DIR)\app\settings" mkdir "$(BUILD_DIR)\app\settings"
	@if not exist "$(BUILD_DIR)\app\scenes" mkdir "$(BUILD_DIR)\app\scenes"
	@if not exist "$(BUILD_DIR)\app\input" mkdir "$(BUILD_DIR)\app\input"
	@if not exist "$(BUILD_DIR)\app\render" mkdir "$(BUILD_DIR)\app\render"
	@if not exist "$(BUILD_DIR)\app\ui" mkdir "$(BUILD_DIR)\app\ui"
	@if not exist "$(BUILD_DIR)\app\camera" mkdir "$(BUILD_DIR)\app\camera"

# Compila arquivos objeto C
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo Compilando $<
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compila arquivos objeto C++
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo Compilando $<
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Linka o executável (usa g++ para linkar C e C++)
$(TARGET): $(OBJS)
	@echo Linkando $(TARGET)
	@$(CXX) $(OBJS) $(LDFLAGS) $(LIBS) -o $(TARGET)
	@echo Copiando assets para build...
	@if not exist "$(BUILD_DIR)\assets" mkdir "$(BUILD_DIR)\assets"
	@xcopy /E /I /Y "assets\*" "$(BUILD_DIR)\assets\" >nul 2>&1 || echo Assets copiados
	@echo Build completo!

# Limpa arquivos compilados
clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@echo Limpeza completa

# Executa o programa
run: $(TARGET)
	@$(TARGET)

# Build com DEBUG: wireframe, etc. Faz clean e rebuild para garantir.
debug: CFLAGS += -DDEBUG
debug: clean $(TARGET)
	@echo Build DEBUG: wireframe em cena, mundo limpo em release.

# Phony targets
.PHONY: all clean run debug
