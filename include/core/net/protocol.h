#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// Tipos de pacotes de rede
typedef enum {
    PKT_JOIN = 0,
    PKT_WELCOME,
    PKT_PLAYER_JOINED,
    PKT_LOBBY_CONFIG,
    PKT_START_RUN,
    PKT_INPUT,
    PKT_SNAPSHOT,
    PKT_EVENT,
    PKT_COUNT
} PacketType;

// Estrutura base de um pacote
typedef struct {
    uint8_t type;
    uint16_t size;
    uint32_t sequence;
} PacketHeader;

// Estrutura de JOIN
typedef struct {
    PacketHeader header;
    char name[32];
} JoinPacket;

// Estrutura de WELCOME
typedef struct {
    PacketHeader header;
    uint32_t playerId;
    uint32_t hostId;
} WelcomePacket;

// Estrutura de START_RUN
typedef struct {
    PacketHeader header;
    uint32_t seed;
    char destination[64];
} StartRunPacket;

// Estrutura de INPUT
typedef struct {
    PacketHeader header;
    uint32_t playerId;
    float moveX;
    float moveY;
    uint32_t buttons;
} InputPacket;

// Estrutura de SNAPSHOT
typedef struct {
    PacketHeader header;
    uint32_t tick;
    // Dados do mundo (serializados)
    uint8_t data[1024];
    uint16_t dataSize;
} SnapshotPacket;

// Estrutura de EVENT
typedef struct {
    PacketHeader header;
    uint8_t eventType;
    uint8_t data[256];
    uint16_t dataSize;
} EventPacket;

#endif // PROTOCOL_H
