#include "core/world/map.h"
#include <string.h>

void Map_Init(Map* map) {
    if (!map) return;
    memset(map, 0, sizeof(Map));
    map->initialized = true;
}

Tile* Map_GetTile(Map* map, int x, int y) {
    if (!map || !Map_IsValidPosition(x, y)) return NULL;
    return &map->tiles[y][x];
}

bool Map_IsValidPosition(int x, int y) {
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}
