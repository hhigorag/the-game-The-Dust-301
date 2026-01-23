#include "core/world/procgen.h"
#include "core/math/rng.h"
#include <stdlib.h>

void Procgen_Init(void) {
    // Nada a fazer por enquanto
}

void Procgen_Generate(Map* map, uint32_t seed) {
    if (!map) return;
    
    Map_Init(map);
    map->seed = seed;
    
    // Gera mapa determinístico baseado na seed
    RNG_Seed(seed);
    
    // Geração simples: cria algumas salas e corredores
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            Tile* tile = Map_GetTile(map, x, y);
            if (!tile) continue;
            
            // Gera padrão simples baseado na seed
            int value = RNG_RandInt(100);
            if (value < 20) {
                tile->type = 1; // Parede
            } else {
                tile->type = 0; // Vazio
            }
            
            tile->explored = false;
            tile->hasLoot = (RNG_RandInt(100) < 2); // 2% de chance de loot
        }
    }
}
