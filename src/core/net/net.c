#include "core/net/net.h"
#include <string.h>
#include <stdlib.h>

// Implementação stub da rede (será implementada com enet depois)
static bool g_initialized = false;
static bool g_connected = false;
static bool g_isHost = false;
static uint32_t g_localPlayerId = 0;

int Net_Init(void) {
    if (g_initialized) return 0;
    g_initialized = true;
    return 0;
}

void Net_Shutdown(void) {
    g_initialized = false;
    g_connected = false;
    g_isHost = false;
}

bool Net_StartServer(uint16_t port) {
    (void)port;
    if (!g_initialized) return false;
    g_isHost = true;
    g_connected = true;
    g_localPlayerId = 1;
    return true;
}

bool Net_Connect(const char* ip, uint16_t port) {
    (void)ip;
    (void)port;
    if (!g_initialized) return false;
    g_isHost = false;
    g_connected = true;
    g_localPlayerId = 2;
    return true;
}

void Net_Disconnect(void) {
    g_connected = false;
    g_isHost = false;
}

bool Net_IsConnected(void) {
    return g_connected;
}

bool Net_IsHost(void) {
    return g_isHost;
}

void Net_Poll(void) {
    // TODO: Implementar polling de pacotes
}

bool Net_SendPacket(const void* data, uint16_t size, bool reliable) {
    (void)data;
    (void)size;
    (void)reliable;
    return false;
}

uint16_t Net_ReceivePacket(void* buffer, uint16_t bufferSize) {
    (void)buffer;
    (void)bufferSize;
    return 0;
}

uint32_t Net_GetLocalPlayerId(void) {
    return g_localPlayerId;
}
