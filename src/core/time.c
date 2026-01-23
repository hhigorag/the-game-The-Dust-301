#include "core/time.h"
#include <windows.h>

static float g_deltaTime = 0.0f;
static float g_totalTime = 0.0f;
static uint64_t g_ticks = 0;
static LARGE_INTEGER g_frequency;
static LARGE_INTEGER g_lastTime;

void Time_Init(void) {
    QueryPerformanceFrequency(&g_frequency);
    QueryPerformanceCounter(&g_lastTime);
    g_deltaTime = 0.0f;
    g_totalTime = 0.0f;
    g_ticks = 0;
}

void Time_Update(void) {
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    
    LONGLONG delta = currentTime.QuadPart - g_lastTime.QuadPart;
    g_deltaTime = (float)delta / (float)g_frequency.QuadPart;
    g_totalTime += g_deltaTime;
    g_ticks++;
    
    g_lastTime = currentTime;
    
    // Limita delta time para evitar saltos grandes
    if (g_deltaTime > 0.1f) {
        g_deltaTime = 0.1f;
    }
}

float Time_GetDeltaTime(void) {
    return g_deltaTime;
}

float Time_GetTotalTime(void) {
    return g_totalTime;
}

uint64_t Time_GetTicks(void) {
    return g_ticks;
}
