#include "core/world/structure_spawner.h"
#include "core/world/world_seed.h"
#include <stdint.h>
#include <stdbool.h>

/* Espaçamento: a cada 10–16 chunks (300–500 m) uma estrutura possível. */
static int32_t NextStructureChunk(int32_t fromChunkZ, uint64_t seed) {
    uint64_t data[2] = { seed, (uint64_t)fromChunkZ };
    uint64_t h = WorldSeed_Hash64(data, sizeof(data));
    int32_t delta = LARGE_STRUCTURE_MIN_CHUNKS + (int32_t)(h % (LARGE_STRUCTURE_MAX_CHUNKS - LARGE_STRUCTURE_MIN_CHUNKS + 1));
    return fromChunkZ + delta;
}

bool StructureSpawner_ShouldSpawnAtChunk(uint64_t worldSeed, int32_t chunkZ) {
    if (chunkZ < LARGE_STRUCTURE_MIN_CHUNKS) return false;
    uint64_t seed = worldSeed ^ 0x53545255ULL;
    int32_t next = LARGE_STRUCTURE_MIN_CHUNKS;
    while (next <= chunkZ) {
        if (next == chunkZ) return true;
        next = NextStructureChunk(next, seed);
    }
    return false;
}

LargeStructureType StructureSpawner_GetTypeAtChunk(uint64_t worldSeed, int32_t chunkZ) {
    uint64_t data[2] = { worldSeed, (uint64_t)chunkZ };
    uint64_t h = WorldSeed_Hash64(data, sizeof(data));
    switch ((int)(h % LARGE_STRUCT_COUNT)) {
        case 1: return LARGE_STRUCT_COMPLEX;
        case 2: return LARGE_STRUCT_BUNKER;
        case 3: return LARGE_STRUCT_TOWER;
        case 4: return LARGE_STRUCT_UNDERGROUND;
        default: return LARGE_STRUCT_COMPLEX;
    }
}
