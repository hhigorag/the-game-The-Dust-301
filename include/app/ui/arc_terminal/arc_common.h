#ifndef ARC_TERMINAL_ARC_COMMON_H
#define ARC_TERMINAL_ARC_COMMON_H

#include <raylib.h>

/* Estados da aplicação */
typedef enum {
    ARC_STATE_BOOT,
    ARC_STATE_LOGIN,
    ARC_STATE_LOADING,
    ARC_STATE_SHELL
} ArcTerminalState;

/* Modos do Shell */
typedef enum {
    ARC_MODE_PRIORITY_MANAGE,
    ARC_MODE_NAVIGATION,
    ARC_MODE_ARC_REPORT
} ArcShellMode;

/* Constantes */
#define ARC_COLOR_GREEN ((Color){51, 255, 51, 255})
#define ARC_MAX_LINES 20
#define ARC_LANDING_STEPS 5
#define ARC_DIVIDER_Y_OFFSET 590
#define ARC_EMERGENCY_PANEL_WIDTH 250

const char* Arc_GetModeTitle(ArcShellMode mode);

#endif
