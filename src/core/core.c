#include "core/core.h"
#include "core/time.h"
#include "core/state/match_state.h"
#include "core/state/game_state.h"
#include "core/state/lobby_state.h"
#include "core/net/net.h"
#include <stdlib.h>
#include <string.h>

// Forward declaration
struct NetSystem;

static bool g_initialized = false;
static NetSystem* g_netSystem = NULL;

int Core_Init(void) {
    if (g_initialized) return 0;
    
    Time_Init();
    GameState_Init(GameState_Get());
    LobbyState_Init(LobbyState_Get());
    MatchState_Set(MATCH_STATE_MENU);
    
    // Inicializa rede
    if (!Net_Init(&g_netSystem)) {
        // Não é fatal, continua sem rede
    }
    
    g_initialized = true;
    return 0;
}

void Core_Tick(float dt) {
    (void)dt; // Usado no futuro
    if (!g_initialized) return;
    
    Time_Update();
    
    // Poll de rede (todo frame)
    if (g_netSystem) {
        Net_Poll(g_netSystem);
    }
    
    // Atualiza simulação apenas se estiver em jogo
    if (MatchState_Get() == MATCH_STATE_INGAME) {
        // TODO: Atualizar gameplay, players, mundo, etc.
    }
}

void Core_Shutdown(void) {
    if (!g_initialized) return;
    
    // Finaliza rede
    if (g_netSystem) {
        Net_Shutdown(g_netSystem);
        g_netSystem = NULL;
    }
    
    g_initialized = false;
}

// Retorna o NetSystem para uso externo
NetSystem* Core_GetNetSystem(void) {
    return g_netSystem;
}

bool Core_IsInitialized(void) {
    return g_initialized;
}
