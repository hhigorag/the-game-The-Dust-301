#include "app/ui/scifi_terminal.h"
#include "core/world/voxel_world.h"
#include "core/world/route.h"
#include "core/world/checkpoint.h"
#include "core/world/zones.h"
#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static Font g_terminalFont = {0};
static bool g_fontLoaded = false;

void SciFiTerminal_Init(SciFiTerminal* terminal) {
    if (!terminal) return;
    memset(terminal, 0, sizeof(SciFiTerminal));
    terminal->visible = false;
    terminal->inputMode = false;
    terminal->interference = 0.0f;
    
    // Carrega fonte (usa fonte padrão se não encontrar)
    if (!g_fontLoaded) {
        g_terminalFont = GetFontDefault();
        g_fontLoaded = true;
    }
}

void SciFiTerminal_Toggle(SciFiTerminal* terminal) {
    if (terminal) {
        terminal->visible = !terminal->visible;
        terminal->inputMode = terminal->visible;
        if (terminal->visible) {
            memset(terminal->inputBuffer, 0, sizeof(terminal->inputBuffer));
            terminal->inputCursor = 0;
        }
    }
}

void SciFiTerminal_AddOutput(SciFiTerminal* terminal, const char* line) {
    if (!terminal || !line) return;
    
    // Move linhas para cima
    if (terminal->outputCount >= 32) {
        for (int i = 0; i < 31; i++) {
            strcpy(terminal->outputLines[i], terminal->outputLines[i + 1]);
        }
        terminal->outputCount = 31;
    }
    
    strncpy(terminal->outputLines[terminal->outputCount], line, 255);
    terminal->outputLines[terminal->outputCount][255] = '\0';
    terminal->outputCount++;
}

void SciFiTerminal_ProcessInput(SciFiTerminal* terminal, int key) {
    if (!terminal || !terminal->inputMode) return;
    
    if (key >= 32 && key <= 126) { // Caracteres imprimíveis
        if (terminal->inputCursor < 255) {
            terminal->inputBuffer[terminal->inputCursor] = (char)key;
            terminal->inputCursor++;
            terminal->inputBuffer[terminal->inputCursor] = '\0';
        }
    } else if (key == KEY_BACKSPACE) {
        if (terminal->inputCursor > 0) {
            terminal->inputCursor--;
            terminal->inputBuffer[terminal->inputCursor] = '\0';
        }
    } else if (key == KEY_ENTER) {
        // Executa comando
        if (terminal->inputCursor > 0) {
            SciFiTerminal_AddOutput(terminal, terminal->inputBuffer);
            // Comando será processado externamente
            terminal->inputCursor = 0;
            terminal->inputBuffer[0] = '\0';
        }
    }
}

void SciFiTerminal_ExecuteCommand(SciFiTerminal* terminal, const char* command,
                                 VoxelWorld* world, Route* route,
                                 CheckpointSystem* checkpoints, ZoneSystem* zones) {
    if (!terminal || !command) return;
    
    char cmd[256];
    strncpy(cmd, command, 255);
    cmd[255] = '\0';
    
    // Converte para maiúsculas
    for (int i = 0; cmd[i]; i++) {
        cmd[i] = (char)toupper(cmd[i]);
    }
    
    // Parse comando
    if (strncmp(cmd, "SET SEED ", 9) == 0) {
        const char* seed = cmd + 9;
        VoxelWorld_SetSeed(world, seed);
        char output[256];
        // Limita tamanho do seed para evitar truncamento
        char seedSafe[200];
        strncpy(seedSafe, seed, sizeof(seedSafe) - 1);
        seedSafe[sizeof(seedSafe) - 1] = '\0';
        snprintf(output, sizeof(output), "> SEED SET: %s", seedSafe);
        SciFiTerminal_AddOutput(terminal, output);
    }
    else if (strncmp(cmd, "SET COURSE ", 11) == 0) {
        float x = 0.0f, z = 0.0f;
        if (sscanf(cmd + 11, "%f %f", &x, &z) == 2) {
            char output[256];
            snprintf(output, sizeof(output), "> COURSE SET: X=%.1f Z=%.1f", x, z);
            SciFiTerminal_AddOutput(terminal, output);
        } else {
            SciFiTerminal_AddOutput(terminal, "> ERROR: Invalid coordinates");
        }
    }
    else if (strcmp(cmd, "SCAN ROUTE") == 0) {
        if (route && route->pointCount > 0) {
            char output[256];
            snprintf(output, sizeof(output), "> ROUTE SCANNED: %d waypoints, length=%.1f", 
                    route->pointCount, route->totalLength);
            SciFiTerminal_AddOutput(terminal, output);
        } else {
            SciFiTerminal_AddOutput(terminal, "> ERROR: No route available");
        }
    }
    else if (strcmp(cmd, "JUMP") == 0) {
        SciFiTerminal_AddOutput(terminal, "> JUMP INITIATED...");
        // TODO: Implementar jump
    }
    else if (strncmp(cmd, "LOAD RADIUS ", 12) == 0) {
        int radius = 0;
        if (sscanf(cmd + 12, "%d", &radius) == 1) {
            char output[256];
            snprintf(output, sizeof(output), "> LOAD RADIUS SET: %d chunks", radius);
            SciFiTerminal_AddOutput(terminal, output);
        } else {
            SciFiTerminal_AddOutput(terminal, "> ERROR: Invalid radius");
        }
    }
    else if (strcmp(cmd, "STATUS") == 0) {
        int32_t loadedChunks = 0, generatingChunks = 0;
        VoxelWorld_GetStats(world, &loadedChunks, &generatingChunks);
        
        char output[256];
        snprintf(output, sizeof(output), "> STATUS: Chunks loaded=%d, generating=%d", 
                loadedChunks, generatingChunks);
        SciFiTerminal_AddOutput(terminal, output);
        
        if (checkpoints) {
            snprintf(output, sizeof(output), "> Checkpoints: %d active", checkpoints->count);
            SciFiTerminal_AddOutput(terminal, output);
        }
        
        if (zones) {
            snprintf(output, sizeof(output), "> Anomaly zones: %d detected", zones->count);
            SciFiTerminal_AddOutput(terminal, output);
        }
    }
    else {
        char output[256];
        snprintf(output, sizeof(output), "> ERROR: Unknown command: %s", command);
        SciFiTerminal_AddOutput(terminal, output);
    }
}

void SciFiTerminal_Update(SciFiTerminal* terminal, float dt) {
    if (!terminal) return;
    (void)dt; // Usado no futuro para animações
    
    // Atualiza interferência (efeito visual)
    terminal->interference *= 0.95f; // Decai gradualmente
}

void SciFiTerminal_Render(SciFiTerminal* terminal) {
    if (!terminal || !terminal->visible) return;
    
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    
    // Fundo semi-transparente
    DrawRectangle(0, 0, width, height, (Color){0, 0, 0, 200});
    
    // Painel do terminal
    int panelX = 50;
    int panelY = 50;
    int panelWidth = width - 100;
    int panelHeight = height - 100;
    
    // Borda
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, GREEN);
    
    // Área de output
    int outputY = panelY + 20;
    for (int i = 0; i < terminal->outputCount && i < 20; i++) {
        Color textColor = GREEN;
        if (terminal->interference > 0.1f) {
            // Efeito de interferência
            textColor.r = (unsigned char)(GREEN.r * (1.0f - terminal->interference));
        }
        DrawTextEx(g_terminalFont, terminal->outputLines[i], 
                   (Vector2){(float)(panelX + 10), (float)(outputY + i * 20)}, 
                   16.0f, 0.0f, textColor);
    }
    
    // Linha de input
    int inputY = panelY + panelHeight - 40;
    DrawTextEx(g_terminalFont, "> ", (Vector2){(float)(panelX + 10), (float)inputY}, 
               16.0f, 0.0f, GREEN);
    
    char inputDisplay[256];
    // Garante que não vai truncar
    size_t bufLen = strlen(terminal->inputBuffer);
    if (bufLen < sizeof(inputDisplay) - 2) {
        snprintf(inputDisplay, sizeof(inputDisplay), "%s_", terminal->inputBuffer);
    } else {
        strncpy(inputDisplay, terminal->inputBuffer, sizeof(inputDisplay) - 2);
        inputDisplay[sizeof(inputDisplay) - 2] = '_';
        inputDisplay[sizeof(inputDisplay) - 1] = '\0';
    }
    DrawTextEx(g_terminalFont, inputDisplay, (Vector2){(float)(panelX + 30), (float)inputY}, 
               16.0f, 0.0f, GREEN);
}
