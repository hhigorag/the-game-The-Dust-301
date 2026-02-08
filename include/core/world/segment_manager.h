#ifndef SEGMENT_MANAGER_H
#define SEGMENT_MANAGER_H

#include "world_config.h"
#include <stdint.h>

/* ============================================================================
 * SEGMENT MANAGER — Macro layer (segmento de roteiro)
 * Cada chunkZ pertence a um segmento dramático.
 * O mundo é procedural; o ritmo é autoral.
 * ============================================================================ */

/* Retorna o tipo de segmento pela distância em metros (eixo Z). */
SegmentType SegmentManager_GetTypeByDistanceM(float worldZ);

/* Retorna o tipo de segmento pelo índice do chunk Z (0..CHUNKS_LONG-1). */
SegmentType SegmentManager_GetTypeByChunkZ(int32_t chunkZ);

/* Retorna o índice do segmento (0..SEGMENT_COUNT-1). ~5 chunks = 1 segmento. */
int32_t SegmentManager_GetSegmentIndex(int32_t chunkZ);

/* Corridor bias: eixo central da nave (em metros). A nave “deriva”.
 * centerX = Noise1D(seed, chunkZ*0.1f) * CORRIDOR_DRIFT_MAX_M
 * Chunks perto de centerX: mais loot/estruturas; longe: hostil. */
float SegmentManager_GetCorridorCenterX(uint64_t worldSeed, int32_t chunkZ);

#endif /* SEGMENT_MANAGER_H */
