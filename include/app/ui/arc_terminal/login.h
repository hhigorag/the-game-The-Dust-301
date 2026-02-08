#ifndef ARC_TERMINAL_LOGIN_H
#define ARC_TERMINAL_LOGIN_H

#include "arc_common.h"
#include <stdbool.h>

typedef struct {
    char* username;
    int* loginLetterCount;
    bool* authError;
} ArcLoginLogicContext;

typedef struct {
    char* username;
    int loginLetterCount;
    bool authError;
    float screenW;
    float screenH;
} ArcLoginRenderContext;

/* Processa uma tecla; retorna true se transicionou para LOADING */
bool Arc_Login_ProcessKey(ArcLoginLogicContext* ctx, int key);
void Arc_Login_Render(ArcLoginRenderContext* ctx, Font font);

#endif
