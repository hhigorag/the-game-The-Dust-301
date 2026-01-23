#include "app/scenes/scene_terminal_lobby.h"
#include "core/state/lobby_state.h"
#include "core/state/match_state.h"
#include "core/net/net.h"
#include "core/net/protocol.h"
#include "app/scenes/scene_manager.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

static bool g_initialized = false;

void Scene_TerminalLobby_Init(void) {
    g_initialized = true;
}

void Scene_TerminalLobby_Update(float dt) {
    (void)dt;
    
    LobbyState* lobby = LobbyState_Get();
    
    // Host pode iniciar o jogo
    if (Net_IsHost() && IsKeyPressed(KEY_ENTER)) {
        if (lobby->allReady && lobby->seed > 0) {
            // Envia PKT_START_RUN
            StartRunPacket pkt = {0};
            pkt.header.type = PKT_START_RUN;
            pkt.seed = lobby->seed;
            strncpy(pkt.destination, lobby->destination, sizeof(pkt.destination) - 1);
            
            Net_SendPacket(&pkt, sizeof(pkt), true);
            
            // Muda para gameplay
            MatchState_Set(MATCH_STATE_INGAME);
            SceneManager_SetScene(SCENE_GAMEPLAY);
        }
    }
}

void Scene_TerminalLobby_Draw(void) {
    LobbyState* lobby = LobbyState_Get();
    
    DrawText("TERMINAL LOBBY", 100, 50, 30, GREEN);
    
    char buf[256];
    snprintf(buf, sizeof(buf), "Players: %d", lobby->playerCount);
    DrawText(buf, 100, 100, 20, WHITE);
    
    for (int i = 0; i < lobby->playerCount; i++) {
        snprintf(buf, sizeof(buf), "- %s %s", 
                 lobby->players[i].name,
                 lobby->players[i].ready ? "[READY]" : "");
        DrawText(buf, 100, 130 + i * 30, 18, WHITE);
    }
    
    if (Net_IsHost()) {
        snprintf(buf, sizeof(buf), "Seed: %u", lobby->seed);
        DrawText(buf, 100, 300, 20, YELLOW);
        
        snprintf(buf, sizeof(buf), "Destination: %s", lobby->destination);
        DrawText(buf, 100, 330, 20, YELLOW);
        
        if (lobby->allReady) {
            DrawText("Press ENTER to Start", 100, 400, 20, GREEN);
        }
    }
}
