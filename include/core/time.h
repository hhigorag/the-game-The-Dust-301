#ifndef TIME_H
#define TIME_H

#include <stdint.h>

// Inicializa o sistema de tempo
void Time_Init(void);

// Atualiza o sistema de tempo (chamado todo frame)
void Time_Update(void);

// Retorna o delta time do frame atual (em segundos)
float Time_GetDeltaTime(void);

// Retorna o tempo total decorrido desde o início (em segundos)
float Time_GetTotalTime(void);

// Retorna o número de ticks desde o início
uint64_t Time_GetTicks(void);

#endif // TIME_H
