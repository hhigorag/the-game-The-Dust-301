#include "app/scenes/scene_manager.h"
#include "app/scenes/scene_menu.h"
#include "app/scenes/scene_gameplay.h"
#include <raylib.h>

static SceneId g_currentScene = SCENE_MENU_MAIN;

static const Scene g_scenes[SCENE_COUNT] = {
    [SCENE_MENU_MAIN] = {
        .Init    = Scene_Menu_Init,
        .Shutdown = Scene_Menu_Shutdown,
        .Update  = Scene_Menu_Update,
        .Draw    = Scene_Menu_Draw,
    },
    [SCENE_GAMEPLAY] = {
        .Init    = Scene_Gameplay_Init,
        .Shutdown = Scene_Gameplay_Shutdown,
        .Update  = Scene_Gameplay_Update,
        .Draw    = Scene_Gameplay_Draw,
    },
};

void SceneManager_Init(void) {
    g_currentScene = SCENE_MENU_MAIN;
    g_scenes[SCENE_MENU_MAIN].Init();
}

void SceneManager_Shutdown(void) {
    g_scenes[g_currentScene].Shutdown();
}

void SceneManager_Change(SceneId next) {
    if (next >= SCENE_COUNT) return;
    g_scenes[g_currentScene].Shutdown();
    g_currentScene = next;
    g_scenes[g_currentScene].Init();
}

SceneId SceneManager_GetCurrent(void) {
    return g_currentScene;
}

void SceneManager_Update(float dt) {
    g_scenes[g_currentScene].Update(dt);
}

void SceneManager_Draw(void) {
    g_scenes[g_currentScene].Draw();
}
