#pragma once
#include <raylib.h>

typedef enum CrosshairType {
    CROSSHAIR_DOT = 0,
    CROSSHAIR_PLUS = 1
} CrosshairType;

typedef enum ActionId {
    ACT_MOVE_FORWARD = 0,
    ACT_MOVE_BACK,
    ACT_MOVE_LEFT,
    ACT_MOVE_RIGHT,
    ACT_JUMP,
    ACT_SPRINT,
    ACT_PAUSE,
    ACT_COUNT
} ActionId;

typedef struct KeyBinding {
    int primary;
    int secondary;
} KeyBinding;

typedef struct Settings {
    float fov;
    float mouseSensitivity;
    CrosshairType crosshair;
    KeyBinding keys[ACT_COUNT];
} Settings;

extern Settings g_settings;

void Settings_SetDefaults(void);
void Settings_Clamp(void);
const char* Settings_ActionName(ActionId a);
const char* Settings_KeyName(int key);
