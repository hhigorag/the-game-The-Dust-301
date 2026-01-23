#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdbool.h>

#define MAP_WIDTH 100
#define MAP_HEIGHT 100

// Estrutura de um tile do mapa
typedef struct {
    uint8_t type; // 0 = vazio, 1 = parede, 2 = porta, etc.
    bool explored;
    bool hasLoot;
} Tile;

// Estrutura do mapa
typedef struct {
    Tile tiles[MAP_WIDTH][MAP_HEIGHT];
    uint32_t seed;
    bool initialized;
} Map;

// Inicializa um mapa vazio
void Map_Init(Map* map);

// Retorna o tile em uma posição
Tile* Map_GetTile(Map* map, int x, int y);

// Verifica se uma posição é válida
bool Map_IsValidPosition(int x, int y);

#endif // MAP_H
