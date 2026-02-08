#include "core/world/streaming_controller.h"
#include "core/world/world_config.h"
#include <stdint.h>

void StreamingController_Init(StreamingController* sc) {
    if (!sc) return;
    sc->currentChunkZ = 0;
    sc->minChunkZ = 0;
    sc->maxChunkZ = STREAM_CHUNKS_AHEAD;
}

void StreamingController_Update(StreamingController* sc, float shipWorldZ) {
    if (!sc) return;
    int32_t chunkZ = WorldZToChunkZ(shipWorldZ);
    sc->currentChunkZ = chunkZ;
    sc->minChunkZ = chunkZ - STREAM_CHUNKS_BEHIND;
    sc->maxChunkZ = chunkZ + STREAM_CHUNKS_AHEAD;
    if (sc->minChunkZ < 0) sc->minChunkZ = 0;
    if (sc->maxChunkZ >= CHUNKS_LONG) sc->maxChunkZ = CHUNKS_LONG - 1;
}

int32_t StreamingController_GetMinChunkZ(const StreamingController* sc) {
    return sc ? sc->minChunkZ : 0;
}

int32_t StreamingController_GetMaxChunkZ(const StreamingController* sc) {
    return sc ? sc->maxChunkZ : STREAM_CHUNKS_AHEAD;
}
