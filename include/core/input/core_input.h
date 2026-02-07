#ifndef CORE_INPUT_H
#define CORE_INPUT_H

#include <stdbool.h>

// ============================================================================
// SISTEMA DE INPUT AGNÓSTICO
// ============================================================================
// Este módulo abstrai o input para que o core não dependa de Raylib.
// A implementação pode usar Raylib, SDL, ou qualquer outra biblioteca.
// ============================================================================

// Estados de teclas
typedef enum {
    KEY_W = 0,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_SPACE,
    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
    KEY_COUNT
} KeyCode;

// Estados de mouse
typedef struct {
    float deltaX;  // Delta X do mouse (movimento horizontal)
    float deltaY;  // Delta Y do mouse (movimento vertical)
    bool locked;   // Mouse travado (FPS mode)?
} MouseState;

// ============================================================================
// API PÚBLICA
// ============================================================================

// Inicializa sistema de input
void CoreInput_Init(void);

// Atualiza estado de input (chamar todo frame)
void CoreInput_Update(void);

// Verifica se tecla está pressionada
bool CoreInput_IsKeyDown(KeyCode key);

// Verifica se tecla foi pressionada neste frame
bool CoreInput_IsKeyPressed(KeyCode key);

// Obtém estado do mouse
MouseState CoreInput_GetMouseState(void);

// Trava/destrava mouse (para modo FPS)
void CoreInput_LockMouse(void);
void CoreInput_UnlockMouse(void);

// Shutdown
void CoreInput_Shutdown(void);

#endif // CORE_INPUT_H
