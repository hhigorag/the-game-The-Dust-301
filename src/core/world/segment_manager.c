#include "core/world/segment_manager.h"
#include "core/world/world_seed.h"
#include <stdint.h>
#include <math.h>

SegmentType SegmentManager_GetTypeByDistanceM(float worldZ) {
    if (worldZ < 0.0f) return SEGMENT_STABLE;
    if (worldZ < (float)ZONE_STABLE_END) return SEGMENT_STABLE;
    if (worldZ < (float)ZONE_RUINS_END) return SEGMENT_URBAN_DENSE;
    if (worldZ < (float)ZONE_OPEN_END) return SEGMENT_INDUSTRIAL;
    if (worldZ < (float)ZONE_UNDERGROUND_END) return SEGMENT_OPEN_DESERT;
    if (worldZ < (float)ZONE_FINAL_END) return SEGMENT_COLLAPSED_STRUCTURE;
    if (worldZ < (float)WORLD_GOAL_Z_M) return SEGMENT_FINAL_CORRIDOR;
    if (worldZ < (float)ZONE_DEAD_END) return SEGMENT_DEAD_ZONE;
    return SEGMENT_DEAD_ZONE;
}

SegmentType SegmentManager_GetTypeByChunkZ(int32_t chunkZ) {
    float worldZ = ChunkZToWorldZ(chunkZ);
    return SegmentManager_GetTypeByDistanceM(worldZ);
}

int32_t SegmentManager_GetSegmentIndex(int32_t chunkZ) {
    if (chunkZ < 0) return 0;
    return chunkZ / CHUNKS_PER_SEGMENT;
}

/* Noise 1D determinístico em [0,1] — amostra em valor inteiro (value noise bruto). */
static float Noise1D_Raw(uint64_t seed, float t) {
    uint64_t h = WorldSeed_Hash64(&seed, sizeof(seed));
    uint64_t v = WorldSeed_Hash64(&t, sizeof(t));
    h ^= v;
    h *= 0x9e3779b97f4a7c15ULL;
    return (float)((h >> 32) & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

/* Smoothstep: C2 em 0 e 1, evita quebras angulares entre nós. */
static float Smoothstep(float x) {
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;
    return x * x * (3.0f - 2.0f * x);
}

/* Noise 1D suavizado: amostra em nós inteiros e interpola com smoothstep.
 * t muda a cada 0.1 por chunk → curva suave a cada ~10 chunks (~160 m). */
static float Noise1D(uint64_t seed, float t) {
    float t0 = floorf(t);
    float t1 = t0 + 1.0f;
    float n0 = Noise1D_Raw(seed, t0);
    float n1 = Noise1D_Raw(seed, t1);
    float frac = t - t0;
    float s = Smoothstep(frac);
    return n0 + (n1 - n0) * s;
}

float SegmentManager_GetCorridorCenterX(uint64_t worldSeed, int32_t chunkZ) {
    float t = (float)chunkZ * 0.1f;
    uint64_t seed = worldSeed ^ (uint64_t)0xDEADBEEF;
    float n = Noise1D(seed, t);
    /* n em [0,1] → [-1, 1] para drift negativo/positivo */
    float signedNoise = (n - 0.5f) * 2.0f;
    return signedNoise * (float)CORRIDOR_DRIFT_MAX_M;
}
