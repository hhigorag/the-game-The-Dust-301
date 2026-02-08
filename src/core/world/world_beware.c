#include "core/world/world_beware.h"
#include "core/world/world_seed.h"
#include "core/world/voxel_world.h"
#include "core/world/segment_manager.h"
#include <string.h>

void WorldBeware_Init(WorldBeware* w, const char* seedString) {
    if (!w) return;
    memset(w, 0, sizeof(WorldBeware));
    w->worldSeed = WorldSeed_StringToU64(seedString ? seedString : "beware-the-dust");
    ThreatSystem_Init(&w->threatSystem, w->worldSeed);
    StreamingController_Init(&w->streamingController);
    w->initialized = true;
}

void WorldBeware_AttachVoxelWorld(WorldBeware* w, struct VoxelWorld* vw) {
    if (!w) return;
    w->voxelWorld = vw;
}

void WorldBeware_Update(WorldBeware* w, float shipX, float shipZ, float overclockUsage, float playerX, float playerZ) {
    if (!w || !w->initialized) return;
    (void)shipX;
    ThreatSystem_Update(&w->threatSystem, shipZ, overclockUsage);
    StreamingController_Update(&w->streamingController, shipZ);
    if (w->voxelWorld) {
        int32_t minZ = StreamingController_GetMinChunkZ(&w->streamingController);
        int32_t maxZ = StreamingController_GetMaxChunkZ(&w->streamingController);
        int32_t currentMacroZ = (int32_t)(shipZ / (float)CHUNK_SIZE_M);
        if (currentMacroZ < 0) currentMacroZ = 0;
        if (currentMacroZ >= CHUNKS_LONG) currentMacroZ = CHUNKS_LONG - 1;
        float corridorCenterX = SegmentManager_GetCorridorCenterX(w->worldSeed, currentMacroZ);
        VoxelWorld_UpdateStreamingFromRange(w->voxelWorld, minZ, maxZ, corridorCenterX, playerX, playerZ);
    }
}

float WorldBeware_GetThreatLevel(const WorldBeware* w) {
    return w ? ThreatSystem_GetLevel(&w->threatSystem) : 0.0f;
}

void WorldBeware_GetStreamRange(const WorldBeware* w, int32_t* outMinChunkZ, int32_t* outMaxChunkZ) {
    if (!w) return;
    if (outMinChunkZ) *outMinChunkZ = StreamingController_GetMinChunkZ(&w->streamingController);
    if (outMaxChunkZ) *outMaxChunkZ = StreamingController_GetMaxChunkZ(&w->streamingController);
}
