#ifndef STRUCTURE_SPAWNER_H
#define STRUCTURE_SPAWNER_H

#include "world_config.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * STRUCTURE SPAWNER — Estruturas grandes semi-raras
 * A cada 300–500 m (10–16 chunks): complexo, bunker, torre caída, base subterrânea.
 * Pré-planejado proceduralmente, não aleatório puro.
 * ============================================================================ */

typedef enum {
    LARGE_STRUCT_NONE = 0,
    LARGE_STRUCT_COMPLEX,
    LARGE_STRUCT_BUNKER,
    LARGE_STRUCT_TOWER,
    LARGE_STRUCT_UNDERGROUND,
    LARGE_STRUCT_COUNT
} LargeStructureType;

/* Retorna se deve spawnar uma estrutura grande neste chunk Z (determinístico). */
bool StructureSpawner_ShouldSpawnAtChunk(uint64_t worldSeed, int32_t chunkZ);

/* Retorna o tipo de estrutura grande para este chunk Z (se ShouldSpawnAtChunk). */
LargeStructureType StructureSpawner_GetTypeAtChunk(uint64_t worldSeed, int32_t chunkZ);

#endif /* STRUCTURE_SPAWNER_H */
