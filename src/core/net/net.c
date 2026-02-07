#include "core/net/net.h"
#include "core/net/protocol.h"
#include "core/state/lobby_state.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ENet: verifica se está disponível
#ifdef USE_ENET
    #include <enet/enet.h>
    #define HAS_ENET 1
#else
    // Stub: compila sem ENet (para desenvolvimento)
    // Para usar ENet: defina USE_ENET e instale ENet em external/enet/
    #define HAS_ENET 0
    
    // Stubs completos do ENet
    typedef uint32_t enet_uint32;
    typedef struct ENetHost ENetHost;
    typedef struct ENetPeer ENetPeer;
    typedef struct ENetPacket {
        size_t dataLength;
        void* data;
    } ENetPacket;
    typedef struct ENetAddress {
        uint32_t host;
        uint16_t port;
    } ENetAddress;
    typedef struct ENetEvent {
        int type;
        ENetPeer* peer;
        ENetPacket* packet;
        size_t dataLength;
    } ENetEvent;
    
    #define ENET_HOST_ANY 0
    #define ENET_PACKET_FLAG_RELIABLE 1
    #define ENET_EVENT_TYPE_CONNECT 1
    #define ENET_EVENT_TYPE_RECEIVE 2
    #define ENET_EVENT_TYPE_DISCONNECT 3
    
    static inline int enet_initialize(void) { return 0; }
    static inline void enet_deinitialize(void) {}
    static inline ENetHost* enet_host_create(ENetAddress* addr, size_t peers, size_t channels, uint32_t inBW, uint32_t outBW) { (void)addr; (void)peers; (void)channels; (void)inBW; (void)outBW; return (ENetHost*)1; }
    static inline void enet_host_destroy(ENetHost* host) { (void)host; }
    static inline ENetPeer* enet_host_connect(ENetHost* host, ENetAddress* addr, size_t channels, uint32_t data) { (void)host; (void)addr; (void)channels; (void)data; return (ENetPeer*)1; }
    static inline int enet_host_service(ENetHost* host, ENetEvent* ev, uint32_t timeout) { (void)host; (void)ev; (void)timeout; return 0; }
    static inline void enet_host_flush(ENetHost* host) { (void)host; }
    static inline void enet_host_broadcast(ENetHost* host, uint8_t channel, ENetPacket* packet) { (void)host; (void)channel; (void)packet; }
    static inline int enet_peer_send(ENetPeer* peer, uint8_t channel, ENetPacket* packet) { (void)peer; (void)channel; (void)packet; return 0; }
    static inline void enet_peer_disconnect(ENetPeer* peer, uint32_t data) { (void)peer; (void)data; }
    static inline ENetPacket* enet_packet_create(const void* data, size_t len, uint32_t flags) { (void)data; (void)len; (void)flags; return (ENetPacket*)calloc(1, sizeof(ENetPacket)); }
    static inline void enet_packet_destroy(ENetPacket* packet) { if (packet) free(packet); }
    static inline int enet_address_set_host(ENetAddress* addr, const char* host) { (void)addr; (void)host; return 0; }
#endif

// Sistema de rede (implementação com ENet)
struct NetSystem {
    NetMode mode;
    ENetHost* host;       // se NETMODE_HOST: servidor; se NETMODE_CLIENT: host client
    ENetPeer* serverPeer; // se client: peer do servidor
    bool connected;
    bool hasClient;
    uint32_t yourId;
};

// Sistema global (para API simplificada)
static NetSystem* g_netSystem = NULL;

static ENetPacket* make_packet(const void* data, uint16_t size, bool reliable) {
#if HAS_ENET
    enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    return enet_packet_create(data, size, flags);
#else
    (void)data; (void)size; (void)reliable;
    ENetPacket* p = (ENetPacket*)calloc(1, sizeof(ENetPacket));
    if (p) {
        p->dataLength = size;
        p->data = malloc(size);
        if (p->data && data) {
            memcpy(p->data, data, size);
        }
    }
    return p;
#endif
}

bool Net_Init(NetSystem** out_ns) {
    if (!out_ns) return false;
    
#if HAS_ENET
    if (enet_initialize() != 0) {
        printf("[NET] enet_initialize failed\n");
        return false;
    }
#else
    printf("[NET] ENet não disponível - usando modo stub\n");
#endif
    
    NetSystem* ns = (NetSystem*)calloc(1, sizeof(NetSystem));
    ns->mode = NETMODE_NONE;
    *out_ns = ns;
    g_netSystem = ns;
    return true;
}

void Net_Shutdown(NetSystem* ns) {
    if (!ns) return;
#if HAS_ENET
    if (ns->host) {
        enet_host_destroy(ns->host);
        ns->host = NULL;
    }
    enet_deinitialize();
#else
    (void)ns->host; // stub
#endif
    free(ns);
    if (g_netSystem == ns) {
        g_netSystem = NULL;
    }
}

bool Net_StartHost(NetSystem* ns, const NetConfig* cfg, int max_clients) {
    if (!ns || !cfg) return false;

#if HAS_ENET
    if (ns->host) {
        enet_host_destroy(ns->host);
        ns->host = NULL;
    }

    ENetAddress address;
    address.port = cfg->port;
    // bind_ip NULL => ENET_HOST_ANY
    if (cfg->bind_ip && cfg->bind_ip[0]) {
        enet_address_set_host(&address, cfg->bind_ip);
    } else {
        address.host = ENET_HOST_ANY;
    }

    ns->host = enet_host_create(
        &address,
        (size_t)max_clients,
        2,              // channels
        0,              // incoming bandwidth (0 = unlimited)
        0               // outgoing bandwidth
    );

    if (!ns->host) {
        printf("[NET] Failed to create server host\n");
        return false;
    }
#else
    (void)max_clients;
    printf("[NET] STUB: HOST mode (ENet não disponível)\n");
#endif

    ns->mode = NETMODE_HOST;
    ns->connected = true;      // host "tá online"
    ns->hasClient = false;
    ns->yourId = 1;            // host id fixo
    
    // OBRIGATÓRIO: Adiciona host ao LobbyState
    LobbyState* lobby = LobbyState_Get();
    LobbyState_AddPlayer(lobby, 1, "Host", true);
    printf("[NET] HOST started on port %u (lobby: %d players)\n", cfg->port, lobby->playerCount);
    return true;
}

bool Net_Connect(NetSystem* ns, const NetConfig* cfg) {
    if (!ns || !cfg || !cfg->connect_ip) return false;

#if HAS_ENET
    if (ns->host) {
        enet_host_destroy(ns->host);
        ns->host = NULL;
    }

    ns->host = enet_host_create(NULL, 1, 2, 0, 0); // client host
    if (!ns->host) {
        printf("[NET] Failed to create client host\n");
        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, cfg->connect_ip);
    address.port = cfg->port;

    ns->serverPeer = enet_host_connect(ns->host, &address, 2, 0);
    if (!ns->serverPeer) {
        printf("[NET] No available peers for connection\n");
        return false;
    }
#else
    printf("[NET] STUB: CLIENT mode (ENet não disponível)\n");
    // Simula conexão após delay
    ns->connected = true;
    ns->yourId = 2;
#endif

    ns->mode = NETMODE_CLIENT;
    if (!HAS_ENET) {
        ns->connected = true; // stub: conecta imediatamente
    } else {
        ns->connected = false;
    }
    ns->hasClient = false;
    if (!HAS_ENET) {
        ns->yourId = 2; // stub
    } else {
        ns->yourId = 0;
    }

    printf("[NET] Connecting to %s:%u...\n", cfg->connect_ip, cfg->port);
    return true;
}

void Net_Disconnect(NetSystem* ns) {
    if (!ns) return;
    if (ns->serverPeer) {
        enet_peer_disconnect(ns->serverPeer, 0);
    }
    ns->connected = false;
    ns->hasClient = false;
}

NetMode Net_GetMode(NetSystem* ns) { 
    return ns ? ns->mode : NETMODE_NONE; 
}

bool Net_IsConnected(NetSystem* ns) { 
    return ns ? ns->connected : false; 
}

bool Net_HasClient(NetSystem* ns) { 
    return ns ? ns->hasClient : false; 
}

uint32_t Net_GetLocalPlayerId(NetSystem* ns) {
    return ns ? ns->yourId : 0;
}

bool Net_SendToHost(NetSystem* ns, const void* data, uint16_t size, bool reliable) {
    if (!ns || ns->mode != NETMODE_CLIENT) return false;
#if HAS_ENET
    if (!ns->serverPeer) return false;
    ENetPacket* p = make_packet(data, size, reliable);
    if (!p) return false;
    return enet_peer_send(ns->serverPeer, 0, p) == 0;
#else
    (void)data; (void)size; (void)reliable;
    printf("[NET] STUB: SendToHost (ENet não disponível)\n");
    return true; // stub: sempre retorna sucesso
#endif
}

bool Net_SendToAll(NetSystem* ns, const void* data, uint16_t size, bool reliable) {
    if (!ns || ns->mode != NETMODE_HOST) return false;
#if HAS_ENET
    if (!ns->host) return false;
    ENetPacket* p = make_packet(data, size, reliable);
    if (!p) return false;
    enet_host_broadcast(ns->host, 0, p);
    return true;
#else
    (void)data; (void)size; (void)reliable;
    printf("[NET] STUB: SendToAll (ENet não disponível)\n");
    return true; // stub: sempre retorna sucesso
#endif
}

static void handle_packet_client(NetSystem* ns, ENetEvent* ev) {
    if (!ev->packet || ev->packet->dataLength < sizeof(PktHeader)) return;
    PktHeader* h = (PktHeader*)ev->packet->data;

    if (h->type == PKT_WELCOME && ev->packet->dataLength >= sizeof(PktWelcome)) {
        PktWelcome* w = (PktWelcome*)ev->packet->data;
        ns->yourId = w->yourId;
        printf("[NET] WELCOME: yourId=%u\n", ns->yourId);
    }
    
    // OBRIGATÓRIO: Processa LOBBY_UPDATE
    if (h->type == PKT_LOBBY_CONFIG && ev->packet->dataLength >= sizeof(PktLobbyUpdate)) {
        PktLobbyUpdate* update = (PktLobbyUpdate*)ev->packet->data;
        LobbyState* lobby = LobbyState_Get();
        
        // Limpa o lobby atual e recria com os dados recebidos do host
        lobby->playerCount = 0;
        for (int i = 0; i < update->playerCount && i < MAX_LOBBY_PLAYERS; i++) {
            // Adiciona player diretamente (sem verificar duplicatas pois estamos recriando)
            if (lobby->playerCount < MAX_LOBBY_PLAYERS) {
                LobbyPlayer* player = &lobby->players[lobby->playerCount++];
                player->id = update->players[i].id;
                player->ready = update->players[i].ready;
                player->isHost = update->players[i].isHost;
                strncpy(player->name, update->players[i].name, sizeof(player->name) - 1);
                player->name[sizeof(player->name) - 1] = '\0';
                
                if (player->isHost) {
                    lobby->hostId = player->id;
                }
            }
        }
        
        // Recalcula allReady
        lobby->allReady = (lobby->playerCount > 0);
        for (int i = 0; i < lobby->playerCount; i++) {
            if (!lobby->players[i].ready) {
                lobby->allReady = false;
                break;
            }
        }
        
        printf("[NET] LOBBY_UPDATE recebido: %d players\n", lobby->playerCount);
    }
}

static void handle_packet_host(NetSystem* ns, ENetEvent* ev) {
    if (!ev->packet || ev->packet->dataLength < sizeof(PktHeader)) return;
    PktHeader* h = (PktHeader*)ev->packet->data;

    if (h->type == PKT_JOIN && ev->packet->dataLength >= sizeof(PktJoin)) {
        PktJoin* j = (PktJoin*)ev->packet->data;
        printf("[NET] Client JOIN name='%s'\n", j->name);

        // OBRIGATÓRIO: Adiciona player ao LobbyState
        LobbyState* lobby = LobbyState_Get();
        uint32_t newPlayerId = 2; // TODO: gerar ID incremental baseado em quantos já tem
        if (LobbyState_AddPlayer(lobby, newPlayerId, j->name, false)) {
            printf("[NET] Player %u '%s' adicionado ao lobby (total: %d)\n", newPlayerId, j->name, lobby->playerCount);
        } else {
            printf("[NET] ERRO: Falha ao adicionar player ao lobby\n");
            return;
        }

        // Responde WELCOME com o ID do player (apenas para o cliente que se conectou)
        PktWelcome w = {0};
        w.h.type = PKT_WELCOME;
        w.h.size = (uint16_t)sizeof(PktWelcome);
        w.yourId = newPlayerId;
        
#if HAS_ENET
        // Envia WELCOME apenas para o cliente específico
        ENetPacket* welcomePkt = make_packet(&w, (uint16_t)sizeof(w), true);
        if (welcomePkt && ev->peer) {
            enet_peer_send(ev->peer, 0, welcomePkt);
            printf("[NET] WELCOME enviado para player %u\n", newPlayerId);
        }
#else
        Net_SendToAll(ns, &w, (uint16_t)sizeof(w), true);
#endif

        // OBRIGATÓRIO: Envia LOBBY_UPDATE para TODOS (incluindo o novo player)
        PktLobbyUpdate update = {0};
        update.h.type = PKT_LOBBY_CONFIG; // Reutiliza LOBBY_CONFIG para update
        update.h.size = (uint16_t)sizeof(PktLobbyUpdate);
        update.playerCount = (uint8_t)lobby->playerCount;
        
        for (int i = 0; i < lobby->playerCount && i < MAX_LOBBY_PLAYERS; i++) {
            update.players[i].id = lobby->players[i].id;
            strncpy(update.players[i].name, lobby->players[i].name, sizeof(update.players[i].name) - 1);
            update.players[i].name[sizeof(update.players[i].name) - 1] = '\0';
            update.players[i].ready = lobby->players[i].ready;
            update.players[i].isHost = lobby->players[i].isHost;
        }
        
        Net_SendToAll(ns, &update, (uint16_t)sizeof(update), true);
        printf("[NET] LOBBY_UPDATE enviado para todos (%d players)\n", lobby->playerCount);
    }
}

void Net_Poll(NetSystem* ns) {
    if (!ns) return;

#if HAS_ENET
    if (!ns->host) return;

    ENetEvent ev;
    while (enet_host_service(ns->host, &ev, 0) > 0) {
        switch (ev.type) {
            case ENET_EVENT_TYPE_CONNECT:
                if (ns->mode == NETMODE_HOST) {
                    ns->hasClient = true;
                    printf("[NET] Client connected\n");
                } else if (ns->mode == NETMODE_CLIENT) {
                    ns->connected = true;
                    printf("[NET] Connected to server\n");

                    // manda JOIN imediatamente
                    PktJoin j = {0};
                    j.h.type = PKT_JOIN;
                    j.h.size = (uint16_t)sizeof(PktJoin);
                    strncpy(j.name, "Player", sizeof(j.name)-1);
                    Net_SendToHost(ns, &j, (uint16_t)sizeof(j), true);
                }
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                if (ns->mode == NETMODE_CLIENT) {
                    handle_packet_client(ns, &ev);
                } else if (ns->mode == NETMODE_HOST) {
                    handle_packet_host(ns, &ev);
                }
                enet_packet_destroy(ev.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                if (ns->mode == NETMODE_HOST) {
                    ns->hasClient = false;
                    printf("[NET] Client disconnected\n");
                } else if (ns->mode == NETMODE_CLIENT) {
                    ns->connected = false;
                    ns->serverPeer = NULL;
                    printf("[NET] Disconnected from server\n");
                }
                break;

            default: break;
        }
    }

    // flush de saída (pra mandar pacotes pendentes)
    enet_host_flush(ns->host);
#else
    // Stub: não faz nada, mas permite compilar
    (void)ns;
#endif
}

// API simplificada (compatibilidade com código existente)
bool Net_StartServer(uint16_t port) {
    if (!g_netSystem) {
        NetSystem* ns = NULL;
        if (!Net_Init(&ns)) return false;
    }
    NetConfig cfg = {0};
    cfg.port = port;
    return Net_StartHost(g_netSystem, &cfg, 8);
}

bool Net_ConnectSimple(const char* ip, uint16_t port) {
    if (!g_netSystem) {
        NetSystem* ns = NULL;
        if (!Net_Init(&ns)) return false;
    }
    NetConfig cfg = {0};
    cfg.connect_ip = ip;
    cfg.port = port;
    return Net_Connect(g_netSystem, &cfg);
}

bool Net_IsHost(void) {
    return g_netSystem && Net_GetMode(g_netSystem) == NETMODE_HOST;
}

uint32_t Net_GetLocalPlayerIdSimple(void) {
    return Net_GetLocalPlayerId(g_netSystem);
}

// Funções stub para compatibilidade (serão removidas depois)
int Net_Init_Old(void) {
    NetSystem* ns = NULL;
    return Net_Init(&ns) ? 0 : 1;
}

void Net_Shutdown_Old(void) {
    if (g_netSystem) {
        Net_Shutdown(g_netSystem);
        g_netSystem = NULL;
    }
}

void Net_Poll_Old(void) {
    if (g_netSystem) {
        Net_Poll(g_netSystem);
    }
}

bool Net_SendPacket(const void* data, uint16_t size, bool reliable) {
    if (!g_netSystem) return false;
    if (Net_GetMode(g_netSystem) == NETMODE_CLIENT) {
        return Net_SendToHost(g_netSystem, data, size, reliable);
    } else {
        return Net_SendToAll(g_netSystem, data, size, reliable);
    }
}

uint16_t Net_ReceivePacket(void* buffer, uint16_t bufferSize) {
    (void)buffer;
    (void)bufferSize;
    // TODO: Implementar fila de recebimento
    return 0;
}

void Net_Disconnect_Old(void) {
    if (g_netSystem) {
        Net_Disconnect(g_netSystem);
    }
}
