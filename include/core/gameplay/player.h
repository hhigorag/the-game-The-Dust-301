#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <stdbool.h>

// Estrutura de um comando de input
typedef struct {
    float moveX;
    float moveY;
    uint32_t buttons;
} InputCmd;

// Aplica um comando de input a um player
void Player_ApplyInput(uint32_t playerId, const InputCmd* cmd, float dt);

// Atualiza um player (física, etc.)
void Player_Update(uint32_t playerId, float dt);

// Retorna a posição X de um player
float Player_GetX(uint32_t playerId);

// Retorna a posição Y de um player
float Player_GetY(uint32_t playerId);

// Retorna o ângulo de um player
float Player_GetAngle(uint32_t playerId);

#endif // PLAYER_H
