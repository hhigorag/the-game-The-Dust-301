#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PLAYERS 8

// Estrutura de um player
typedef struct {
    uint32_t id;
    float x, y;
    float angle;
    bool connected;
    char name[32];
} Player;

// Estrutura do estado da nave
typedef struct {
    float energy;
    float fuel;
    float oxygen;
    bool emergency;
} ShipState;

// Estrutura principal do estado do jogo
typedef struct {
    Player players[MAX_PLAYERS];
    int playerCount;
    ShipState ship;
    uint32_t mapSeed;
    float gameTime;
    bool initialized;
} GameState;

// Inicializa o GameState
void GameState_Init(GameState* state);

// Reseta o GameState para um novo jogo
void GameState_Reset(GameState* state, uint32_t seed);

// Retorna o GameState global
GameState* GameState_Get(void);

#endif // GAME_STATE_H
