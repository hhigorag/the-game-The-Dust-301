#include "app/app.h"
#include "core/core.h"
#include "core/net/net.h"
#include "core/time.h"
#include "app/scenes/scene_manager.h"
#include <stdio.h>

int main(void) {
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    
    // Inicializa App (janela, áudio)
    if (App_Init(screenWidth, screenHeight, "Exploration Co-op") != 0) {
        fprintf(stderr, "Erro ao inicializar App\n");
        return 1;
    }
    
    // Inicializa Core (lógica do jogo)
    if (Core_Init() != 0) {
        fprintf(stderr, "Erro ao inicializar Core\n");
        App_Shutdown();
        return 1;
    }
    
    // Inicializa Rede
    if (Net_Init() != 0) {
        fprintf(stderr, "Erro ao inicializar Rede\n");
        Core_Shutdown();
        App_Shutdown();
        return 1;
    }
    
    // Inicializa SceneManager
    SceneManager_Init();
    
    // Loop principal único
    while (!App_ShouldClose()) {
        // Calcula dt
        float dt = Time_GetDeltaTime();
        
        // Poll de rede (todo frame)
        Net_Poll();
        
        // Atualiza Core (lógica do jogo)
        Core_Tick(dt);
        
        // Atualiza Scene (UI + input)
        SceneManager_Update(dt);
        
        // Renderiza
        SceneManager_Draw();
    }
    
    // Finaliza tudo
    SceneManager_SetScene(SCENE_MENU); // Garante que não há referências pendentes
    Net_Shutdown();
    Core_Shutdown();
    App_Shutdown();
    
    return 0;
}
