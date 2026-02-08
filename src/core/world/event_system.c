#include "core/world/event_system.h"
#include "core/world/world_seed.h"
#include <stdint.h>

uint64_t EventSystem_GetEventSeed(uint64_t worldSeed, int32_t segmentIndex) {
    uint64_t data[2] = { worldSeed, (uint64_t)segmentIndex };
    return WorldSeed_Hash64(data, sizeof(data));
}

SegmentEventType EventSystem_GetEventForSegment(uint64_t worldSeed, int32_t segmentIndex) {
    uint64_t seed = EventSystem_GetEventSeed(worldSeed, segmentIndex);
    /* Mapeia seed a tipo de evento; ~25% chance de evento por segmento */
    uint64_t h = seed % 100;
    if (h < 25) return EVENT_STORM;
    if (h < 50) return EVENT_RADAR_GLITCH;
    if (h < 70) return EVENT_SILENCE_ZONE;
    if (h < 85) return EVENT_GRAVITY_FAIL;
    return EVENT_NONE;
}
