#include "core/world/world_generator.h"
#include "core/world/voxel_world.h"
#include "core/world/route.h"
#include "core/world/checkpoint.h"
#include "core/world/zones.h"
#include "core/world/structures.h"
#include "core/math/rng.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

WorldGenerator* WorldGenerator_Create(const char* seedString) {
    WorldGenerator* gen = (WorldGenerator*)calloc(1, sizeof(WorldGenerator));
    if (!gen) return NULL;

    gen->world = VoxelWorld_Create(seedString);
    if (!gen->world) {
        free(gen);
        return NULL;
    }

    gen->route = Route_Create();
    if (!gen->route) {
        VoxelWorld_Destroy(gen->world);
        free(gen);
        return NULL;
    }

    gen->checkpoints = CheckpointSystem_Create();
    gen->zones = ZoneSystem_Create();
    gen->structures = StructureSystem_Create();

    if (!gen->checkpoints || !gen->zones || !gen->structures) {
        WorldGenerator_Destroy(gen);
        return NULL;
    }

    gen->config.mapMinX = -200;
    gen->config.mapMaxX = 200;
    gen->config.mapMinZ = -200;
    gen->config.mapMaxZ = 200;
    gen->config.mapHeight = 128;
    gen->config.spawnX = 0.0f;
    gen->config.spawnY = 64.0f;
    gen->config.spawnZ = 0.0f;
    gen->config.numZones = 5;
    gen->config.numStructures = 20;
    gen->config.checkpointMinDist = 50.0f;

    gen->initialized = true;

    return gen;
}

void WorldGenerator_Destroy(WorldGenerator* gen) {
    if (!gen) return;

    if (gen->structures) StructureSystem_Destroy(gen->structures);
    if (gen->zones) ZoneSystem_Destroy(gen->zones);
    if (gen->checkpoints) CheckpointSystem_Destroy(gen->checkpoints);
    if (gen->route) Route_Destroy(gen->route);
    if (gen->world) VoxelWorld_Destroy(gen->world);

    free(gen);
}

void WorldGenerator_SetConfig(WorldGenerator* gen, const WorldGenConfig* config) {
    if (!gen || !config) return;
    gen->config = *config;
}

void WorldGenerator_Generate(WorldGenerator* gen) {
    if (!gen || !gen->initialized) return;

    uint64_t seed = VoxelWorld_GetSeedU64(gen->world);
    RNG_Seed((uint32_t)seed);

    const int32_t floorY = 63;
    const int32_t innerMinX = gen->config.mapMinX + 1;
    const int32_t innerMaxX = gen->config.mapMaxX - 1;
    const int32_t innerMinZ = gen->config.mapMinZ + 1;
    const int32_t innerMaxZ = gen->config.mapMaxZ - 1;

    /* 1) Borda preta (limite do mapa) */
    for (int32_t x = gen->config.mapMinX; x <= gen->config.mapMaxX; x++) {
        for (int32_t z = gen->config.mapMinZ; z <= gen->config.mapMaxZ; z++) {
            if (x == gen->config.mapMinX || x == gen->config.mapMaxX ||
                z == gen->config.mapMinZ || z == gen->config.mapMaxZ) {
                for (int32_t y = 0; y < gen->config.mapHeight; y++) {
                    Voxel v = {BLOCK_BLACK, 0};
                    VoxelWorld_SetBlock(gen->world, x, y, z, v);
                }
            }
        }
    }

    /* 2) Grande piso único (chão) em todo o interior – só chão, sem elevações */
    for (int32_t x = innerMinX; x <= innerMaxX; x++) {
        for (int32_t z = innerMinZ; z <= innerMaxZ; z++) {
            Voxel v = {BLOCK_TERRAIN, 0};
            VoxelWorld_SetBlock(gen->world, x, floorY, z, v);
        }
    }

    /* 3) Spawn no centro: pés no chão, sem plataforma vermelha */
    int32_t cx = (innerMinX + innerMaxX) / 2;
    int32_t cz = (innerMinZ + innerMaxZ) / 2;
    gen->config.spawnX = (float)cx;
    gen->config.spawnY = (float)(floorY + 1);
    gen->config.spawnZ = (float)cz;
    /* Opcional: um único bloco vermelho no chão como marcador */
    {
        Voxel v = {BLOCK_RED, 0};
        VoxelWorld_SetBlock(gen->world, cx, floorY, cz, v);
    }

    /* 4) Prédios ao redor (apenas ao longo das bordas internas) */
    StructureSystem_Generate(gen->structures, gen->world, gen->route,
                             innerMinX, innerMaxX, innerMinZ, innerMaxZ,
                             gen->config.numStructures);
    StructureSystem_ApplyToWorld(gen->structures, gen->world);

    /* Rotas/checkpoints/zones desativados para mapa “só chão + prédios” */
    (void)gen->route;
    (void)gen->checkpoints;
    (void)gen->zones;
}

void WorldGenerator_GetSpawn(const WorldGenerator* gen, float* outX, float* outY, float* outZ) {
    if (!gen || !outX || !outY || !outZ) return;
    *outX = gen->config.spawnX;
    *outY = gen->config.spawnY;
    *outZ = gen->config.spawnZ;
}

VoxelWorld* WorldGenerator_GetWorld(WorldGenerator* gen) {
    return gen ? gen->world : NULL;
}

Route* WorldGenerator_GetRoute(WorldGenerator* gen) {
    return gen ? gen->route : NULL;
}

CheckpointSystem* WorldGenerator_GetCheckpoints(WorldGenerator* gen) {
    return gen ? gen->checkpoints : NULL;
}

ZoneSystem* WorldGenerator_GetZones(WorldGenerator* gen) {
    return gen ? gen->zones : NULL;
}

StructureSystem* WorldGenerator_GetStructures(WorldGenerator* gen) {
    return gen ? gen->structures : NULL;
}
