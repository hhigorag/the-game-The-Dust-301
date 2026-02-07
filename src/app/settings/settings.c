#include "app/settings/settings.h"
#include <string.h>
#include <stdio.h>

Settings g_settings;

void Settings_SetDefaults(void) {
    g_settings.fov = 75.0f;
    g_settings.mouseSensitivity = 0.25f;
    g_settings.crosshair = CROSSHAIR_DOT;

    g_settings.keys[ACT_MOVE_FORWARD] = (KeyBinding){ KEY_W, 0 };
    g_settings.keys[ACT_MOVE_BACK]    = (KeyBinding){ KEY_S, 0 };
    g_settings.keys[ACT_MOVE_LEFT]    = (KeyBinding){ KEY_A, 0 };
    g_settings.keys[ACT_MOVE_RIGHT]   = (KeyBinding){ KEY_D, 0 };
    g_settings.keys[ACT_JUMP]         = (KeyBinding){ KEY_SPACE, 0 };
    g_settings.keys[ACT_SPRINT]       = (KeyBinding){ KEY_LEFT_SHIFT, 0 };
    g_settings.keys[ACT_PAUSE]        = (KeyBinding){ KEY_ESCAPE, 0 };
}

void Settings_Clamp(void) {
    if (g_settings.fov < 60.0f) g_settings.fov = 60.0f;
    if (g_settings.fov > 110.0f) g_settings.fov = 110.0f;
    if (g_settings.mouseSensitivity < 0.05f) g_settings.mouseSensitivity = 0.05f;
    if (g_settings.mouseSensitivity > 1.0f) g_settings.mouseSensitivity = 1.0f;
}

const char* Settings_ActionName(ActionId a) {
    switch (a) {
        case ACT_MOVE_FORWARD: return "Move Forward";
        case ACT_MOVE_BACK:    return "Move Back";
        case ACT_MOVE_LEFT:    return "Move Left";
        case ACT_MOVE_RIGHT:   return "Move Right";
        case ACT_JUMP:         return "Jump";
        case ACT_SPRINT:       return "Sprint";
        case ACT_PAUSE:        return "Pause";
        default: return "?";
    }
}

static const struct { int key; const char* name; } s_keyNames[] = {
    { KEY_W, "W" }, { KEY_A, "A" }, { KEY_S, "S" }, { KEY_D, "D" },
    { KEY_SPACE, "SPACE" }, { KEY_LEFT_SHIFT, "LSHIFT" }, { KEY_ESCAPE, "ESC" },
    { KEY_ENTER, "ENTER" }, { KEY_BACKSPACE, "BACKSPACE" },
    { KEY_UP, "UP" }, { KEY_DOWN, "DOWN" }, { KEY_LEFT, "LEFT" }, { KEY_RIGHT, "RIGHT" },
    { KEY_F1, "F1" }, { KEY_F2, "F2" }, { KEY_F3, "F3" }, { KEY_N, "N" },
    { KEY_Q, "Q" }, { KEY_E, "E" }, { KEY_LEFT_CONTROL, "LCTRL" },
    { 0, NULL }
};

const char* Settings_KeyName(int key) {
    if (key == 0) return "-";
    for (int i = 0; s_keyNames[i].name != NULL; i++) {
        if (s_keyNames[i].key == key) return s_keyNames[i].name;
    }
    static char buf[12];
    snprintf(buf, sizeof(buf), "%d", key);
    return buf;
}
