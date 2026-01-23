# Compilador
CC = gcc

# Diretórios
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
EXTERNAL_DIR = external

# Caminho do raylib (ajuste se necessário)
RAYLIB_DIR = C:/Users/Higor/Desktop/raylib-5.5_win64_mingw-w64
RAYLIB_INCLUDE = $(RAYLIB_DIR)/include
RAYLIB_LIB = $(RAYLIB_DIR)/lib

# Flags de compilação
CFLAGS = -Wall -Wextra -std=c99 -O2 -D_DEFAULT_SOURCE
INCLUDES = -I$(INCLUDE_DIR) -I$(INCLUDE_DIR)/core -I$(INCLUDE_DIR)/app -I$(RAYLIB_INCLUDE)
LDFLAGS = -L$(RAYLIB_LIB)

# Bibliotecas
LIBS = -lraylib -lwinmm -lgdi32 -luser32

# Arquivos fonte Core
CORE_SRC = $(SRC_DIR)/core/core.c \
           $(SRC_DIR)/core/time.c \
           $(SRC_DIR)/core/state/match_state.c \
           $(SRC_DIR)/core/state/game_state.c \
           $(SRC_DIR)/core/state/lobby_state.c \
           $(SRC_DIR)/core/math/rng.c \
           $(SRC_DIR)/core/world/map.c \
           $(SRC_DIR)/core/world/procgen.c \
           $(SRC_DIR)/core/gameplay/player.c \
           $(SRC_DIR)/core/net/net.c

# Arquivos fonte App
APP_SRC = $(SRC_DIR)/app/app.c \
          $(SRC_DIR)/app/scenes/scene_manager.c \
          $(SRC_DIR)/app/scenes/scene_menu.c \
          $(SRC_DIR)/app/scenes/scene_terminal_lobby.c \
          $(SRC_DIR)/app/scenes/scene_gameplay.c \
          $(SRC_DIR)/app/input/input.c

# Todos os arquivos fonte
ALL_SRC = $(SRC_DIR)/main.c $(CORE_SRC) $(APP_SRC)

# Arquivos objeto
OBJS = $(ALL_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

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
	@if not exist "$(BUILD_DIR)\core\gameplay" mkdir "$(BUILD_DIR)\core\gameplay"
	@if not exist "$(BUILD_DIR)\core\net" mkdir "$(BUILD_DIR)\core\net"
	@if not exist "$(BUILD_DIR)\app" mkdir "$(BUILD_DIR)\app"
	@if not exist "$(BUILD_DIR)\app\scenes" mkdir "$(BUILD_DIR)\app\scenes"
	@if not exist "$(BUILD_DIR)\app\input" mkdir "$(BUILD_DIR)\app\input"

# Compila arquivos objeto
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo Compilando $<
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Linka o executável
$(TARGET): $(OBJS)
	@echo Linkando $(TARGET)
	@$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $(TARGET)
	@echo Build completo!

# Limpa arquivos compilados
clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@echo Limpeza completa

# Executa o programa
run: $(TARGET)
	@$(TARGET)

# Phony targets
.PHONY: all clean run
