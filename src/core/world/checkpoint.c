#include "core/world/checkpoint.h"
#include "core/world/voxel_world.h"
#include "core/world/route.h"
#include "core/math/rng.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_CAPACITY 16

CheckpointSystem* CheckpointSystem_Create(void) {
    CheckpointSystem* system = (CheckpointSystem*)calloc(1, sizeof(CheckpointSystem));
    if (!system) return NULL;
    
    system->capacity = INITIAL_CAPACITY;
    system->checkpoints = (Checkpoint*)calloc(system->capacity, sizeof(Checkpoint));
    if (!system->checkpoints) {
        free(system);
        return NULL;
    }
    
    system->count = 0;
    system->nextId = 1;
    
    return system;
}

void CheckpointSystem_Destroy(CheckpointSystem* system) {
    if (system) {
        if (system->checkpoints) free(system->checkpoints);
        free(system);
    }
}

void CheckpointSystem_Add(CheckpointSystem* system, float x, float y, float z, float radius) {
    if (!system || !system->checkpoints) return;
    
    if (system->count >= system->capacity) {
        system->capacity *= 2;
        system->checkpoints = (Checkpoint*)realloc(system->checkpoints, system->capacity * sizeof(Checkpoint));
        if (!system->checkpoints) return;
    }
    
    Checkpoint* cp = &system->checkpoints[system->count];
    cp->x = x;
    cp->y = y;
    cp->z = z;
    cp->radius = radius;
    cp->activated = false;
    cp->id = system->nextId++;
    
    system->count++;
}

void CheckpointSystem_GenerateAlongRoute(CheckpointSystem* system, const Route* route,
                                        VoxelWorld* world, float minDistance) {
    if (!system || !route || !world) return;
    
    // Limpa checkpoints existentes
    system->count = 0;
    system->nextId = 1;
    
    // Gera checkpoints ao longo da rota
    float currentDist = 0.0f;
    float routeLength = route->totalLength;
    
    while (currentDist < routeLength) {
        float t = currentDist / routeLength;
        if (t > 1.0f) t = 1.0f;
        
        float x, z, width;
        Route_GetPositionAt(route, t, &x, &z, &width);
        
        float y = 64.0f; // Altura base
        float radius = 8.0f; // Raio de ativação
        
        CheckpointSystem_Add(system, x, y, z, radius);
        
        currentDist += minDistance;
    }
}

Checkpoint* CheckpointSystem_CheckActivation(CheckpointSystem* system, float playerX, float playerY, float playerZ) {
    if (!system) return NULL;
    
    for (int i = 0; i < system->count; i++) {
        Checkpoint* cp = &system->checkpoints[i];
        if (cp->activated) continue;
        
        float dx = playerX - cp->x;
        float dy = playerY - cp->y;
        float dz = playerZ - cp->z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        
        if (dist <= cp->radius) {
            cp->activated = true;
            return cp;
        }
    }
    
    return NULL;
}

void CheckpointSystem_ApplyToWorld(CheckpointSystem* system, VoxelWorld* world) {
    if (!system || !world) return;
    
    for (int i = 0; i < system->count; i++) {
        Checkpoint* cp = &system->checkpoints[i];
        
        int32_t centerX = (int32_t)cp->x;
        int32_t centerY = (int32_t)cp->y;
        int32_t centerZ = (int32_t)cp->z;
        int32_t radius = (int32_t)cp->radius;
        
        // Pinta área do checkpoint (violeta)
        for (int32_t dy = -radius; dy <= radius; dy++) {
            for (int32_t dz = -radius; dz <= radius; dz++) {
                for (int32_t dx = -radius; dx <= radius; dx++) {
                    if (dx * dx + dy * dy + dz * dz <= radius * radius) {
                        Voxel voxel = {BLOCK_VIOLET, 0};
                        VoxelWorld_SetBlock(world, centerX + dx, centerY + dy, centerZ + dz, voxel);
                    }
                }
            }
        }
    }
}

Checkpoint* CheckpointSystem_GetNearest(CheckpointSystem* system, float x, float y, float z) {
    if (!system || system->count == 0) return NULL;
    
    Checkpoint* nearest = NULL;
    float minDist = INFINITY;
    
    for (int i = 0; i < system->count; i++) {
        Checkpoint* cp = &system->checkpoints[i];
        float dx = x - cp->x;
        float dy = y - cp->y;
        float dz = z - cp->z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        
        if (dist < minDist) {
            minDist = dist;
            nearest = cp;
        }
    }
    
    return nearest;
}
