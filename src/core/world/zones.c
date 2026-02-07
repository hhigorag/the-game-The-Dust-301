#include "core/world/zones.h"
#include "core/world/voxel_world.h"
#include "core/world/route.h"
#include "core/math/rng.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_CAPACITY 16

ZoneSystem* ZoneSystem_Create(void) {
    ZoneSystem* system = (ZoneSystem*)calloc(1, sizeof(ZoneSystem));
    if (!system) return NULL;
    
    system->capacity = INITIAL_CAPACITY;
    system->zones = (AnomalyZone*)calloc(system->capacity, sizeof(AnomalyZone));
    if (!system->zones) {
        free(system);
        return NULL;
    }
    
    system->count = 0;
    system->nextId = 1;
    
    return system;
}

void ZoneSystem_Destroy(ZoneSystem* system) {
    if (system) {
        if (system->zones) free(system->zones);
        free(system);
    }
}

void ZoneSystem_Add(ZoneSystem* system, float x, float y, float z, float radius, float intensity) {
    if (!system || !system->zones) return;
    
    if (system->count >= system->capacity) {
        system->capacity *= 2;
        system->zones = (AnomalyZone*)realloc(system->zones, system->capacity * sizeof(AnomalyZone));
        if (!system->zones) return;
    }
    
    AnomalyZone* zone = &system->zones[system->count];
    zone->centerX = x;
    zone->centerY = y;
    zone->centerZ = z;
    zone->radius = radius;
    zone->intensity = intensity;
    zone->id = system->nextId++;
    zone->active = true;
    
    system->count++;
}

void ZoneSystem_Generate(ZoneSystem* system, VoxelWorld* world, const Route* route,
                        int32_t mapMinX, int32_t mapMaxX,
                        int32_t mapMinZ, int32_t mapMaxZ,
                        int32_t count) {
    if (!system || !world || !route) return;
    
    // Limpa zonas existentes
    system->count = 0;
    system->nextId = 1;
    
    uint64_t seed = VoxelWorld_GetSeedU64(world);
    RNG_Seed((uint32_t)(seed + 12345)); // Offset para variar da rota
    
    float mapWidth = (float)(mapMaxX - mapMinX);
    float mapDepth = (float)(mapMaxZ - mapMinZ);
    
    for (int i = 0; i < count; i++) {
        // Gera posição aleatória
        float x = mapMinX + RNG_RandFloat() * mapWidth;
        float z = mapMinZ + RNG_RandFloat() * mapDepth;
        float y = 64.0f; // Altura base
        
        // Raio variável
        float radius = 16.0f + RNG_RandFloatRange(0.0f, 32.0f);
        
        // Intensidade variável
        float intensity = 0.3f + RNG_RandFloat() * 0.7f;
        
        ZoneSystem_Add(system, x, y, z, radius, intensity);
    }
}

float ZoneSystem_GetIntensityAt(ZoneSystem* system, float x, float y, float z) {
    if (!system) return 0.0f;
    
    float totalIntensity = 0.0f;
    
    for (int i = 0; i < system->count; i++) {
        AnomalyZone* zone = &system->zones[i];
        if (!zone->active) continue;
        
        float dx = x - zone->centerX;
        float dy = y - zone->centerY;
        float dz = z - zone->centerZ;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        
        if (dist <= zone->radius) {
            // Intensidade diminui com a distância
            float factor = 1.0f - (dist / zone->radius);
            totalIntensity += zone->intensity * factor;
        }
    }
    
    return totalIntensity > 1.0f ? 1.0f : totalIntensity;
}

bool ZoneSystem_IsInZone(ZoneSystem* system, float x, float y, float z) {
    if (!system) return false;
    
    for (int i = 0; i < system->count; i++) {
        AnomalyZone* zone = &system->zones[i];
        if (!zone->active) continue;
        
        float dx = x - zone->centerX;
        float dy = y - zone->centerY;
        float dz = z - zone->centerZ;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        
        if (dist <= zone->radius) {
            return true;
        }
    }
    
    return false;
}

void ZoneSystem_ApplyToWorld(ZoneSystem* system, VoxelWorld* world) {
    if (!system || !world) return;
    
    for (int i = 0; i < system->count; i++) {
        AnomalyZone* zone = &system->zones[i];
        if (!zone->active) continue;
        
        int32_t centerX = (int32_t)zone->centerX;
        int32_t centerY = (int32_t)zone->centerY;
        int32_t centerZ = (int32_t)zone->centerZ;
        int32_t radius = (int32_t)zone->radius;
        
        // Centro verde
        Voxel centerVoxel = {BLOCK_GREEN, 0};
        VoxelWorld_SetBlock(world, centerX, centerY, centerZ, centerVoxel);
        
        // Área roxa ao redor
        for (int32_t dy = -radius; dy <= radius; dy++) {
            for (int32_t dz = -radius; dz <= radius; dz++) {
                for (int32_t dx = -radius; dx <= radius; dx++) {
                    float dist = sqrtf((float)(dx * dx + dy * dy + dz * dz));
                    if (dist <= radius && dist > 1.0f) {
                        Voxel voxel = {BLOCK_PURPLE, 0};
                        VoxelWorld_SetBlock(world, centerX + dx, centerY + dy, centerZ + dz, voxel);
                    }
                }
            }
        }
    }
}
