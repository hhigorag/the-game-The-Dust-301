#include "core/input/core_input.h"

#ifdef USE_RAYLIB
#include <raylib.h>
#endif

// ============================================================================
// IMPLEMENTAÇÃO COM RAYLIB (pode ser trocada por SDL, etc)
// ============================================================================

static bool g_keys[KEY_COUNT] = {0};
static bool g_keysPrev[KEY_COUNT] = {0};
static MouseState g_mouseState = {0};
static bool g_initialized = false;

#ifdef USE_RAYLIB
// Mapeamento KeyCode -> Raylib KEY
static int GetRaylibKey(KeyCode key) {
    switch (key) {
        case KEY_W: return KEY_W;
        case KEY_A: return KEY_A;
        case KEY_S: return KEY_S;
        case KEY_D: return KEY_D;
        case KEY_SPACE: return KEY_SPACE;
        case KEY_LEFT_SHIFT: return KEY_LEFT_SHIFT;
        case KEY_LEFT_CONTROL: return KEY_LEFT_CONTROL;
        default: return 0;
    }
}
#endif

void CoreInput_Init(void) {
    if (g_initialized) return;
    
    // Inicializa estados
    for (int i = 0; i < KEY_COUNT; i++) {
        g_keys[i] = false;
        g_keysPrev[i] = false;
    }
    
    g_mouseState.deltaX = 0.0f;
    g_mouseState.deltaY = 0.0f;
    g_mouseState.locked = false;
    
    g_initialized = true;
}

void CoreInput_Update(void) {
    if (!g_initialized) return;
    
#ifdef USE_RAYLIB
    // Salva estado anterior
    for (int i = 0; i < KEY_COUNT; i++) {
        g_keysPrev[i] = g_keys[i];
    }
    
    // Atualiza estado atual
    for (int i = 0; i < KEY_COUNT; i++) {
        int rlKey = GetRaylibKey(i);
        g_keys[i] = IsKeyDown(rlKey);
    }
    
    // Atualiza mouse
    Vector2 delta = GetMouseDelta();
    g_mouseState.deltaX = delta.x;
    g_mouseState.deltaY = delta.y;
    g_mouseState.locked = IsCursorHidden();
#else
    // Stub: implementar com outra biblioteca se necessário
    // Por enquanto, mantém estados zerados
#endif
}

bool CoreInput_IsKeyDown(KeyCode key) {
    if (key >= KEY_COUNT) return false;
    return g_keys[key];
}

bool CoreInput_IsKeyPressed(KeyCode key) {
    if (key >= KEY_COUNT) return false;
    // Pressionada neste frame mas não no anterior
    return g_keys[key] && !g_keysPrev[key];
}

MouseState CoreInput_GetMouseState(void) {
    return g_mouseState;
}

void CoreInput_LockMouse(void) {
#ifdef USE_RAYLIB
    HideCursor();
    DisableCursor();
#endif
    g_mouseState.locked = true;
}

void CoreInput_UnlockMouse(void) {
#ifdef USE_RAYLIB
    ShowCursor();
    EnableCursor();
#endif
    g_mouseState.locked = false;
}

void CoreInput_Shutdown(void) {
    g_initialized = false;
}
