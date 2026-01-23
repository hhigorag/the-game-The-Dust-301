#include "core/state/game_state.h"
#include <string.h>
#include <stdlib.h>

static GameState g_gameState = {0};

void GameState_Init(GameState* state) {
    if (!state) return;
    memset(state, 0, sizeof(GameState));
    state->ship.energy = 100.0f;
    state->ship.fuel = 100.0f;
    state->ship.oxygen = 100.0f;
    state->ship.emergency = false;
    state->playerCount = 0;
    state->initialized = true;
}

void GameState_Reset(GameState* state, uint32_t seed) {
    if (!state) return;
    GameState_Init(state);
    state->mapSeed = seed;
    state->gameTime = 0.0f;
}

GameState* GameState_Get(void) {
    return &g_gameState;
}
