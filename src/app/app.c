#include "app/app.h"
#include <raylib.h>

static bool g_initialized = false;

int App_Init(int width, int height, const char* title) {
    if (g_initialized) return 0;
    
    InitWindow(width, height, title);
    SetTargetFPS(60);
    InitAudioDevice();
    
    // Desabilita ESC fechar a janela automaticamente
    // Agora ESC é tratado manualmente em cada cena
    SetExitKey(0);
    
    g_initialized = true;
    return 0;
}

void App_Shutdown(void) {
    if (!g_initialized) return;
    
    CloseAudioDevice();
    CloseWindow();
    g_initialized = false;
}

bool App_ShouldClose(void) {
    return WindowShouldClose();
}

bool App_IsInitialized(void) {
    return g_initialized;
}
