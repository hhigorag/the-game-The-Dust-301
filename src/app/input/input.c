#include "app/input/input.h"
#include "app/input/bindings.h"
#include <raylib.h>

static InputCmd g_currentCmd = {0};

void Input_Update(void) {
    g_currentCmd.moveX = 0.0f;
    g_currentCmd.moveY = 0.0f;
    g_currentCmd.buttons = 0;
    
    // Movimento
    if (IsKeyDown(KEY_MOVE_RIGHT)) {
        g_currentCmd.moveX = 1.0f;
    }
    if (IsKeyDown(KEY_MOVE_LEFT)) {
        g_currentCmd.moveX = -1.0f;
    }
    if (IsKeyDown(KEY_MOVE_UP)) {
        g_currentCmd.moveY = -1.0f;
    }
    if (IsKeyDown(KEY_MOVE_DOWN)) {
        g_currentCmd.moveY = 1.0f;
    }
    
    // Botões
    if (IsKeyPressed(KEY_INTERACT)) {
        g_currentCmd.buttons |= 1;
    }
}

InputCmd Input_GetCommand(void) {
    return g_currentCmd;
}

bool Input_IsKeyPressed(int key) {
    return IsKeyDown(key);
}

bool Input_IsKeyJustPressed(int key) {
    return IsKeyPressed(key);
}
