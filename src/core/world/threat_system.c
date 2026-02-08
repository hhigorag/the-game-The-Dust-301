#include "core/world/threat_system.h"
#include "core/world/world_seed.h"
#include <math.h>

void ThreatSystem_Init(ThreatSystem* ts, uint64_t worldSeed) {
    if (!ts) return;
    ts->worldSeed = worldSeed;
    ts->baseLevel = 0.1f;
    ts->escalationFactor = 0.00015f;  /* por metro */
    ts->noiseFactor = 0.05f;
    ts->overclockUsage = 0.0f;
    ts->lastLevel = 0.0f;
}

static float EngineNoise(uint64_t seed, float z) {
    float t = z * 0.02f;
    uint64_t h = WorldSeed_Hash64(&seed, sizeof(seed));
    uint64_t v = WorldSeed_Hash64(&t, sizeof(t));
    h ^= v;
    h *= 0x9e3779b97f4a7c15ULL;
    return (float)((h >> 32) & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

float ThreatSystem_Update(ThreatSystem* ts, float shipZ, float overclockUsage) {
    if (!ts) return 0.0f;
    ts->overclockUsage = overclockUsage < 0.0f ? 0.0f : (overclockUsage > 1.0f ? 1.0f : overclockUsage);
    float distance = shipZ < 0.0f ? 0.0f : shipZ;
    float noise = EngineNoise(ts->worldSeed, distance);
    float level = ts->baseLevel
        + distance * ts->escalationFactor
        + noise * ts->noiseFactor
        + ts->overclockUsage * 0.2f;
    ts->lastLevel = level < 0.0f ? 0.0f : level;
    return ts->lastLevel;
}

float ThreatSystem_GetLevel(const ThreatSystem* ts) {
    return ts ? ts->lastLevel : 0.0f;
}
