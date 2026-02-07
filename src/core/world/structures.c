#include "core/world/structures.h"
#include "core/world/voxel_world.h"
#include "core/world/route.h"
#include "core/math/rng.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_CAPACITY 32

StructureSystem* StructureSystem_Create(void) {
    StructureSystem* system = (StructureSystem*)calloc(1, sizeof(StructureSystem));
    if (!system) return NULL;
    
    system->capacity = INITIAL_CAPACITY;
    system->structures = (Structure*)calloc(system->capacity, sizeof(Structure));
    if (!system->structures) {
        free(system);
        return NULL;
    }
    
    system->count = 0;
    system->nextId = 1;
    
    return system;
}

void StructureSystem_Destroy(StructureSystem* system) {
    if (system) {
        if (system->structures) free(system->structures);
        free(system);
    }
}

void StructureSystem_Add(StructureSystem* system, float x, float y, float z,
                       int32_t width, int32_t height, int32_t depth, StructureType type) {
    if (!system || !system->structures) return;
    
    if (system->count >= system->capacity) {
        system->capacity *= 2;
        system->structures = (Structure*)realloc(system->structures, system->capacity * sizeof(Structure));
        if (!system->structures) return;
    }
    
    Structure* s = &system->structures[system->count];
    s->x = x;
    s->y = y;
    s->z = z;
    s->width = width;
    s->height = height;
    s->depth = depth;
    s->type = type;
    s->id = system->nextId++;
    
    system->count++;
}

void StructureSystem_Generate(StructureSystem* system, VoxelWorld* world, const Route* route,
                             int32_t mapMinX, int32_t mapMaxX,
                             int32_t mapMinZ, int32_t mapMaxZ,
                             int32_t count) {
    if (!system || !world) return;
    
    (void)route;
    
    system->count = 0;
    system->nextId = 1;
    
    uint64_t seed = VoxelWorld_GetSeedU64(world);
    RNG_Seed((uint32_t)(seed + 54321));
    
    /* Borda interna (onde ficam os prédios) */
    int32_t innerMinX = mapMinX + 2;
    int32_t innerMaxX = mapMaxX - 2;
    int32_t innerMinZ = mapMinZ + 2;
    int32_t innerMaxZ = mapMaxZ - 2;
    int32_t floorY = 63;
    int32_t pad = 20; /* distância mínima do centro */
    
    for (int i = 0; i < count; i++) {
        /* Coloca prédios ao redor: perto das bordas, longe do centro */
        int32_t x, z;
        int side = RNG_RandInt(4);
        switch (side) {
            case 0: x = innerMinX + RNG_RandInt(24); z = innerMinZ + RNG_RandInt(innerMaxZ - innerMinZ); break;
            case 1: x = innerMaxX - RNG_RandInt(24); z = innerMinZ + RNG_RandInt(innerMaxZ - innerMinZ); break;
            case 2: x = innerMinX + RNG_RandInt(innerMaxX - innerMinX); z = innerMinZ + RNG_RandInt(24); break;
            default: x = innerMinX + RNG_RandInt(innerMaxX - innerMinX); z = innerMaxZ - RNG_RandInt(24); break;
        }
        /* Evita centro (spawn) */
        if (x > -pad && x < pad && z > -pad && z < pad) {
            if (x >= 0) x += pad; else x -= pad;
            if (z >= 0) z += pad; else z -= pad;
        }
        
        int32_t w = 6 + RNG_RandInt(10);
        int32_t h = 8 + RNG_RandInt(16);
        int32_t d = 6 + RNG_RandInt(10);
        StructureType type = (StructureType)(RNG_RandInt(STRUCTURE_COUNT));
        
        StructureSystem_Add(system, (float)x, (float)floorY, (float)z, w, h, d, type);
    }
}

void StructureSystem_ApplyToWorld(StructureSystem* system, VoxelWorld* world) {
    if (!system || !world) return;
    
    for (int i = 0; i < system->count; i++) {
        Structure* s = &system->structures[i];
        
        int32_t startX = (int32_t)s->x;
        int32_t startY = (int32_t)s->y;
        int32_t startZ = (int32_t)s->z;
        
        // Preenche estrutura com blocos cinza
        for (int32_t dy = 0; dy < s->height; dy++) {
            for (int32_t dz = 0; dz < s->depth; dz++) {
                for (int32_t dx = 0; dx < s->width; dx++) {
                    // Paredes externas apenas (simplificado)
                    if (dx == 0 || dx == s->width - 1 ||
                        dz == 0 || dz == s->depth - 1 ||
                        dy == 0 || dy == s->height - 1) {
                        Voxel voxel = {BLOCK_GRAY, 0};
                        VoxelWorld_SetBlock(world, startX + dx, startY + dy, startZ + dz, voxel);
                    }
                }
            }
        }
    }
}

bool StructureSystem_CheckCollision(StructureSystem* system, float x, float y, float z) {
    if (!system) return false;
    
    for (int i = 0; i < system->count; i++) {
        Structure* s = &system->structures[i];
        
        if (x >= s->x && x < s->x + s->width &&
            y >= s->y && y < s->y + s->height &&
            z >= s->z && z < s->z + s->depth) {
            return true;
        }
    }
    
    return false;
}
