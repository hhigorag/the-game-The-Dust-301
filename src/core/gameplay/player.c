#include "core/gameplay/player.h"
#include "core/state/game_state.h"
#include <math.h>

void Player_ApplyInput(uint32_t playerId, const InputCmd* cmd, float dt) {
    if (!cmd) return;
    
    GameState* state = GameState_Get();
    if (!state || !state->initialized) return;
    
    // Encontra o player
    Player* player = NULL;
    for (int i = 0; i < state->playerCount; i++) {
        if (state->players[i].id == playerId) {
            player = &state->players[i];
            break;
        }
    }
    
    if (!player || !player->connected) return;
    
    // Aplica movimento
    float speed = 100.0f; // pixels por segundo
    player->x += cmd->moveX * speed * dt;
    player->y += cmd->moveY * speed * dt;
    
    // Atualiza ângulo se houver movimento
    if (cmd->moveX != 0.0f || cmd->moveY != 0.0f) {
        player->angle = atan2f(cmd->moveY, cmd->moveX);
    }
}

void Player_Update(uint32_t playerId, float dt) {
    // Atualizações de física, etc.
    (void)playerId;
    (void)dt;
}

float Player_GetX(uint32_t playerId) {
    GameState* state = GameState_Get();
    if (!state) return 0.0f;
    
    for (int i = 0; i < state->playerCount; i++) {
        if (state->players[i].id == playerId) {
            return state->players[i].x;
        }
    }
    return 0.0f;
}

float Player_GetY(uint32_t playerId) {
    GameState* state = GameState_Get();
    if (!state) return 0.0f;
    
    for (int i = 0; i < state->playerCount; i++) {
        if (state->players[i].id == playerId) {
            return state->players[i].y;
        }
    }
    return 0.0f;
}

float Player_GetAngle(uint32_t playerId) {
    GameState* state = GameState_Get();
    if (!state) return 0.0f;
    
    for (int i = 0; i < state->playerCount; i++) {
        if (state->players[i].id == playerId) {
            return state->players[i].angle;
        }
    }
    return 0.0f;
}
