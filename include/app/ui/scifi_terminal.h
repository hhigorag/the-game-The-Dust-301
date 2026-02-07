#ifndef SCIFI_TERMINAL_H
#define SCIFI_TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct VoxelWorld VoxelWorld;
typedef struct Route Route;
typedef struct CheckpointSystem CheckpointSystem;
typedef struct ZoneSystem ZoneSystem;

// Comandos do terminal
typedef enum {
    CMD_NONE,
    CMD_SET_SEED,
    CMD_SET_COURSE,
    CMD_SCAN_ROUTE,
    CMD_JUMP,
    CMD_LOAD_RADIUS,
    CMD_STATUS,
    CMD_COUNT
} TerminalCommand;

// Estado do terminal
typedef struct {
    bool visible;           // Terminal visível?
    bool inputMode;         // Modo de input ativo?
    char inputBuffer[256];  // Buffer de input
    int32_t inputCursor;   // Posição do cursor
    char outputLines[32][256]; // Linhas de output
    int32_t outputCount;    // Número de linhas
    TerminalCommand lastCommand;
    float interference;     // Interferência de anomalia (0.0 a 1.0)
} SciFiTerminal;

// Inicializa o terminal
void SciFiTerminal_Init(SciFiTerminal* terminal);

// Atualiza o terminal
void SciFiTerminal_Update(SciFiTerminal* terminal, float dt);

// Processa input do terminal
void SciFiTerminal_ProcessInput(SciFiTerminal* terminal, int key);

// Executa um comando
void SciFiTerminal_ExecuteCommand(SciFiTerminal* terminal, const char* command,
                                  VoxelWorld* world, Route* route,
                                  CheckpointSystem* checkpoints, ZoneSystem* zones);

// Adiciona linha de output
void SciFiTerminal_AddOutput(SciFiTerminal* terminal, const char* line);

// Renderiza o terminal
void SciFiTerminal_Render(SciFiTerminal* terminal);

// Toggle visibilidade
void SciFiTerminal_Toggle(SciFiTerminal* terminal);

#endif // SCIFI_TERMINAL_H
