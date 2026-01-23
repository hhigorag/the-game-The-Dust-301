#ifndef PROCGEN_H
#define PROCGEN_H

#include <stdint.h>
#include "map.h"

// Gera um mapa procedural baseado em uma seed
// Esta função DEVE ser determinística (mesma seed = mesmo mapa)
void Procgen_Generate(Map* map, uint32_t seed);

// Inicializa o sistema de geração procedural
void Procgen_Init(void);

#endif // PROCGEN_H
