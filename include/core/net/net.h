#ifndef NET_H
#define NET_H

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

// Inicializa o sistema de rede
int Net_Init(void);

// Finaliza o sistema de rede
void Net_Shutdown(void);

// Inicia um servidor (host)
bool Net_StartServer(uint16_t port);

// Conecta a um servidor (cliente)
bool Net_Connect(const char* ip, uint16_t port);

// Desconecta
void Net_Disconnect(void);

// Verifica se está conectado
bool Net_IsConnected(void);

// Verifica se é o host
bool Net_IsHost(void);

// Poll de pacotes (chamado todo frame)
void Net_Poll(void);

// Envia um pacote
bool Net_SendPacket(const void* data, uint16_t size, bool reliable);

// Recebe um pacote (retorna tamanho ou 0 se não houver)
uint16_t Net_ReceivePacket(void* buffer, uint16_t bufferSize);

// Retorna o ID do player local
uint32_t Net_GetLocalPlayerId(void);

#endif // NET_H
