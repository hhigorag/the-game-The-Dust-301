#include "core/state/lobby_state.h"
#include <string.h>
#include <stdlib.h>

static LobbyState g_lobbyState = {0};

void LobbyState_Init(LobbyState* state) {
    if (!state) return;
    memset(state, 0, sizeof(LobbyState));
    state->seed = 0;
    state->destination[0] = '\0';
    state->initialized = true;
}

LobbyState* LobbyState_Get(void) {
    return &g_lobbyState;
}

bool LobbyState_AddPlayer(LobbyState* state, uint32_t id, const char* name, bool isHost) {
    if (!state || state->playerCount >= MAX_LOBBY_PLAYERS) return false;
    
    // Verifica se já existe
    for (int i = 0; i < state->playerCount; i++) {
        if (state->players[i].id == id) return false;
    }
    
    LobbyPlayer* player = &state->players[state->playerCount++];
    player->id = id;
    player->ready = false;
    player->isHost = isHost;
    strncpy(player->name, name ? name : "Player", sizeof(player->name) - 1);
    player->name[sizeof(player->name) - 1] = '\0';
    
    if (isHost) {
        state->hostId = id;
    }
    
    return true;
}

void LobbyState_RemovePlayer(LobbyState* state, uint32_t id) {
    if (!state) return;
    
    for (int i = 0; i < state->playerCount; i++) {
        if (state->players[i].id == id) {
            // Move todos os players após este para frente
            for (int j = i; j < state->playerCount - 1; j++) {
                state->players[j] = state->players[j + 1];
            }
            state->playerCount--;
            break;
        }
    }
}

bool LobbyState_SetSeed(LobbyState* state, uint32_t seed, uint32_t requesterId) {
    if (!state || requesterId != state->hostId) return false;
    state->seed = seed;
    return true;
}

bool LobbyState_SetDestination(LobbyState* state, const char* dest, uint32_t requesterId) {
    if (!state || requesterId != state->hostId) return false;
    if (dest) {
        strncpy(state->destination, dest, sizeof(state->destination) - 1);
        state->destination[sizeof(state->destination) - 1] = '\0';
    }
    return true;
}

void LobbyState_SetReady(LobbyState* state, uint32_t id, bool ready) {
    if (!state) return;
    
    for (int i = 0; i < state->playerCount; i++) {
        if (state->players[i].id == id) {
            state->players[i].ready = ready;
            break;
        }
    }
    
    // Verifica se todos estão prontos
    state->allReady = true;
    for (int i = 0; i < state->playerCount; i++) {
        if (!state->players[i].ready) {
            state->allReady = false;
            break;
        }
    }
}
