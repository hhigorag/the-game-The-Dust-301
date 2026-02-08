#ifndef ARC_TERMINAL_SHELL_H
#define ARC_TERMINAL_SHELL_H

#include "arc_common.h"
#include <raylib.h>
#include <stdbool.h>

/* Contexto completo do shell (estado, histórico, comandos, etc.) */
typedef struct ArcShellContext ArcShellContext;

ArcShellContext* Arc_Shell_Create(void);
void Arc_Shell_Destroy(ArcShellContext* ctx);

void Arc_Shell_Reset(ArcShellContext* ctx, const char* username);

/* Update: mouse em coords locais do overlay (0..screenW, 0..screenH); dt em segundos */
void Arc_Shell_Update(ArcShellContext* ctx, float screenW, float screenH,
                      int mouseLocalX, int mouseLocalY, bool mouseLeftPressed, float mouseWheel, float dt);

/* Processa uma tecla (char ou KEY_*); username/cpuName/totalRam/resW/resH para ExecuteCommand. */
void Arc_Shell_ProcessKey(ArcShellContext* ctx, int key,
    const char* username, const char* cpuName, unsigned long long totalRam, int resW, int resH);

void Arc_Shell_Render(ArcShellContext* ctx, Font font, Texture2D logo, bool logoOk,
                      const char* username, const char* cpuName, unsigned long long totalRam,
                      float screenW, float screenH);

/* Callbacks que o shell precisa (AddToTerminal, AddToLog, ExecuteCommand) são internos. */

#endif
