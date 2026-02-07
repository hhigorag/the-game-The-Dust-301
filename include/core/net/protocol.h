#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_LOBBY_PLAYERS 8

#pragma pack(push, 1)

// Tipos de pacotes de rede
typedef enum {
    PKT_JOIN = 1,
    PKT_WELCOME = 2,
    PKT_PING = 3,
    PKT_PLAYER_JOINED = 4,
    PKT_LOBBY_CONFIG = 5,
    PKT_START_RUN = 6,
    PKT_INPUT = 7,
    PKT_SNAPSHOT = 8,
    PKT_EVENT = 9
} PacketType;

// Header base de pacote
typedef struct {
    uint8_t type;        // PacketType
    uint16_t size;       // bytes do pacote inteiro (inclui header)
} PktHeader;

// JOIN: client -> host
typedef struct {
    PktHeader h;
    char name[24];
} PktJoin;

// WELCOME: host -> client
typedef struct {
    PktHeader h;
    uint32_t yourId;
} PktWelcome;

// LOBBY_CONFIG: host -> client
typedef struct {
    PktHeader h;
    uint32_t seed;
    char destination[64];
} PktLobbyConfig;

// LOBBY_UPDATE: host -> client (atualiza lista de players)
typedef struct {
    PktHeader h;
    uint8_t playerCount;
    struct {
        uint32_t id;
        char name[32];
        bool ready;
        bool isHost;
    } players[MAX_LOBBY_PLAYERS];
} PktLobbyUpdate;

// START_RUN: host -> client
typedef struct {
    PktHeader h;
    uint32_t seed;
    char destination[64];
} PktStartRun;

// INPUT: client -> host
typedef struct {
    PktHeader h;
    uint32_t playerId;
    float moveX;
    float moveY;
    uint32_t buttons;
} PktInput;

// SNAPSHOT: host -> client
typedef struct {
    PktHeader h;
    uint32_t tick;
    uint8_t data[1024];
    uint16_t dataSize;
} PktSnapshot;

// EVENT: host -> client
typedef struct {
    PktHeader h;
    uint8_t eventType;
    uint8_t data[256];
    uint16_t dataSize;
} PktEvent;

#pragma pack(pop)

#endif // PROTOCOL_H
