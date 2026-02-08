#ifndef WORLD_BEWARE_H
#define WORLD_BEWARE_H

#include "world_config.h"
#include "segment_manager.h"
#include "threat_system.h"
#include "event_system.h"
#include "streaming_controller.h"
#include "structure_spawner.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * WORLD — Beware The Dust
 * Agrega: SegmentManager, ChunkManager, ThreatSystem, EventSystem,
 *         StructureSpawner, StreamingController.
 * A nave alimenta todos eles.
 * ============================================================================ */

struct VoxelWorld;

typedef struct WorldBeware {
    uint64_t worldSeed;
    ThreatSystem threatSystem;
    StreamingController streamingController;
    /* ChunkManager / VoxelWorld: usar VoxelWorld existente via ponteiro externo */
    struct VoxelWorld* voxelWorld;
    bool initialized;
} WorldBeware;

/* Inicializa o mundo com seed (string ou já convertida). */
void WorldBeware_Init(WorldBeware* w, const char* seedString);

/* Anexa o VoxelWorld ao WorldBeware (streaming/geração usa este mundo). */
void WorldBeware_AttachVoxelWorld(WorldBeware* w, struct VoxelWorld* vw);

/* Atualiza streaming, threat e estado com base na posição da nave (metros).
 * playerX, playerZ: posição do jogador em metros; nunca descarrega o chunk do player. */
void WorldBeware_Update(WorldBeware* w, float shipX, float shipZ, float overclockUsage, float playerX, float playerZ);

/* Retorna o nível de ameaça atual. */
float WorldBeware_GetThreatLevel(const WorldBeware* w);

/* Retorna o range de chunks Z a manter carregados. */
void WorldBeware_GetStreamRange(const WorldBeware* w, int32_t* outMinChunkZ, int32_t* outMaxChunkZ);

#endif /* WORLD_BEWARE_H */
