#include "app/scenes/scene_gameplay.h"
#include "core/state/game_state.h"
#include "core/state/match_state.h"
#include "core/gameplay/player.h"
#include "core/net/net.h"
#include "app/input/input.h"
#include <raylib.h>

static bool g_initialized = false;

void Scene_Gameplay_Init(void) {
    g_initialized = true;
}

void Scene_Gameplay_Update(float dt) {
    (void)dt;
    
    // Captura input e envia para o host
    InputCmd cmd = Input_GetCommand();
    
    if (Net_IsHost()) {
        // Host aplica input diretamente
        uint32_t localId = Net_GetLocalPlayerId();
        Player_ApplyInput(localId, &cmd, dt);
    } else {
        // Cliente envia input para o host
        InputPacket pkt = {0};
        pkt.header.type = PKT_INPUT;
        pkt.playerId = Net_GetLocalPlayerId();
        pkt.moveX = cmd.moveX;
        pkt.moveY = cmd.moveY;
        pkt.buttons = cmd.buttons;
        
        Net_SendPacket(&pkt, sizeof(pkt), false);
    }
}

void Scene_Gameplay_Draw(void) {
    GameState* state = GameState_Get();
    
    DrawText("GAMEPLAY", 10, 10, 20, WHITE);
    
    // Desenha players
    for (int i = 0; i < state->playerCount; i++) {
        if (state->players[i].connected) {
            float x = state->players[i].x;
            float y = state->players[i].y;
            DrawCircle(x, y, 10, GREEN);
        }
    }
}
