#include "app/scenes/scene_manager.h"
#include "app/scenes/scene_menu.h"
#include "app/scenes/scene_terminal_lobby.h"
#include "app/scenes/scene_gameplay.h"
#include <raylib.h>

static SceneType g_currentScene = SCENE_MENU;

void SceneManager_Init(void) {
    g_currentScene = SCENE_MENU;
    Scene_Menu_Init();
    Scene_TerminalLobby_Init();
    Scene_Gameplay_Init();
}

void SceneManager_SetScene(SceneType scene) {
    if (scene >= SCENE_COUNT) return;
    g_currentScene = scene;
}

SceneType SceneManager_GetCurrentScene(void) {
    return g_currentScene;
}

void SceneManager_Update(float dt) {
    switch (g_currentScene) {
        case SCENE_MENU:
            Scene_Menu_Update(dt);
            break;
        case SCENE_TERMINAL_LOBBY:
            Scene_TerminalLobby_Update(dt);
            break;
        case SCENE_GAMEPLAY:
            Scene_Gameplay_Update(dt);
            break;
        default:
            break;
    }
}

void SceneManager_Draw(void) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    switch (g_currentScene) {
        case SCENE_MENU:
            Scene_Menu_Draw();
            break;
        case SCENE_TERMINAL_LOBBY:
            Scene_TerminalLobby_Draw();
            break;
        case SCENE_GAMEPLAY:
            Scene_Gameplay_Draw();
            break;
        default:
            break;
    }
    
    EndDrawing();
}
