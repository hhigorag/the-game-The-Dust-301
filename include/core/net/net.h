#ifndef NET_H
#define NET_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

// Modo de rede
typedef enum {
    NETMODE_NONE = 0,
    NETMODE_HOST,
    NETMODE_CLIENT
} NetMode;

// Configuração de rede
typedef struct {
    const char* bind_ip;     // NULL = qualquer interface (host)
    uint16_t port;           // 27015
    const char* connect_ip;  // client: "127.0.0.1" ou "26.x.x.x"
} NetConfig;

// Sistema de rede (opaco)
typedef struct NetSystem NetSystem;

// Inicializa o sistema de rede
bool Net_Init(NetSystem** out_ns);

// Finaliza o sistema de rede
void Net_Shutdown(NetSystem* ns);

// Inicia servidor (host)
bool Net_StartHost(NetSystem* ns, const NetConfig* cfg, int max_clients);

// Conecta a um servidor (cliente)
bool Net_Connect(NetSystem* ns, const NetConfig* cfg);

// Desconecta
void Net_Disconnect(NetSystem* ns);

// Poll de pacotes (chamado TODO frame)
void Net_Poll(NetSystem* ns);

// Getters
NetMode Net_GetMode(NetSystem* ns);
bool Net_IsConnected(NetSystem* ns); // client conectado ao host?
bool Net_HasClient(NetSystem* ns);   // host tem algum cliente conectado?
uint32_t Net_GetLocalPlayerId(NetSystem* ns);

// Enviar pacotes
bool Net_SendToHost(NetSystem* ns, const void* data, uint16_t size, bool reliable);
bool Net_SendToAll(NetSystem* ns, const void* data, uint16_t size, bool reliable);

// API simplificada (compatibilidade com código existente)
bool Net_StartServer(uint16_t port);
bool Net_ConnectSimple(const char* ip, uint16_t port);
bool Net_IsHost(void);
uint32_t Net_GetLocalPlayerIdSimple(void);

#endif // NET_H
