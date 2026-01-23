#ifndef LOBBY_STATE_H
#define LOBBY_STATE_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_LOBBY_PLAYERS 8

// Estrutura de um player no lobby
typedef struct {
    uint32_t id;
    char name[32];
    bool ready;
    bool isHost;
} LobbyPlayer;

// Estrutura do estado do lobby
typedef struct {
    LobbyPlayer players[MAX_LOBBY_PLAYERS];
    int playerCount;
    uint32_t hostId;
    uint32_t seed;
    char destination[64];
    bool allReady;
    bool initialized;
} LobbyState;

// Inicializa o LobbyState
void LobbyState_Init(LobbyState* state);

// Retorna o LobbyState global
LobbyState* LobbyState_Get(void);

// Adiciona um player ao lobby
bool LobbyState_AddPlayer(LobbyState* state, uint32_t id, const char* name, bool isHost);

// Remove um player do lobby
void LobbyState_RemovePlayer(LobbyState* state, uint32_t id);

// Define o seed (apenas host)
bool LobbyState_SetSeed(LobbyState* state, uint32_t seed, uint32_t requesterId);

// Define o destino (apenas host)
bool LobbyState_SetDestination(LobbyState* state, const char* dest, uint32_t requesterId);

// Marca um player como ready
void LobbyState_SetReady(LobbyState* state, uint32_t id, bool ready);

#endif // LOBBY_STATE_H
