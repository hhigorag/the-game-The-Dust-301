#include "core/core.h"
#include "core/time.h"
#include "core/state/match_state.h"
#include "core/state/game_state.h"
#include "core/state/lobby_state.h"
#include <stdlib.h>
#include <string.h>

static bool g_initialized = false;

int Core_Init(void) {
    if (g_initialized) return 0;
    
    Time_Init();
    GameState_Init(GameState_Get());
    LobbyState_Init(LobbyState_Get());
    MatchState_Set(MATCH_STATE_MENU);
    
    g_initialized = true;
    return 0;
}

void Core_Tick(float dt) {
    (void)dt; // Usado no futuro
    if (!g_initialized) return;
    
    Time_Update();
    
    // Atualiza simulação apenas se estiver em jogo
    if (MatchState_Get() == MATCH_STATE_INGAME) {
        // TODO: Atualizar gameplay, players, mundo, etc.
    }
}

void Core_Shutdown(void) {
    if (!g_initialized) return;
    g_initialized = false;
}

bool Core_IsInitialized(void) {
    return g_initialized;
}
