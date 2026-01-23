#include "app/scenes/scene_menu.h"
#include "core/net/net.h"
#include "app/scenes/scene_manager.h"
#include <raylib.h>

static bool g_initialized = false;

void Scene_Menu_Init(void) {
    g_initialized = true;
}

void Scene_Menu_Update(float dt) {
    (void)dt;
    
    // Lógica do menu (baseada no código original, mas adaptada)
    if (IsKeyPressed(KEY_ENTER)) {
        // Host
        if (Net_StartServer(7777)) {
            SceneManager_SetScene(SCENE_TERMINAL_LOBBY);
        }
    }
    
    if (IsKeyPressed(KEY_ESCAPE)) {
        // Quit
    }
}

void Scene_Menu_Draw(void) {
    DrawText("MENU - Host/Join/Options/Quit", 100, 100, 20, WHITE);
    DrawText("Press ENTER to Host", 100, 150, 20, GREEN);
    DrawText("Press ESC to Quit", 100, 200, 20, RED);
}
