#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration
typedef struct NetSystem NetSystem;

// Inicializa o sistema Core
int Core_Init(void);

// Atualiza a simulação do jogo (chamado todo frame)
void Core_Tick(float dt);

// Finaliza o sistema Core
void Core_Shutdown(void);

// Verifica se o Core está inicializado
bool Core_IsInitialized(void);

// Retorna o NetSystem (para uso externo)
NetSystem* Core_GetNetSystem(void);

#endif // CORE_H
