#include "core/world/route.h"
#include "core/world/voxel_world.h"
#include "core/world/world_seed.h"
#include "core/math/rng.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_CAPACITY 32

Route* Route_Create(void) {
    Route* route = (Route*)calloc(1, sizeof(Route));
    if (!route) return NULL;
    
    route->capacity = INITIAL_CAPACITY;
    route->points = (RoutePoint*)calloc(route->capacity, sizeof(RoutePoint));
    if (!route->points) {
        free(route);
        return NULL;
    }
    
    route->pointCount = 0;
    route->totalLength = 0.0f;
    route->initialized = true;
    
    return route;
}

void Route_Destroy(Route* route) {
    if (route) {
        if (route->points) free(route->points);
        free(route);
    }
}

void Route_AddPoint(Route* route, float x, float z, float width) {
    if (!route || !route->points) return;
    
    if (route->pointCount >= route->capacity) {
        route->capacity *= 2;
        route->points = (RoutePoint*)realloc(route->points, route->capacity * sizeof(RoutePoint));
        if (!route->points) return;
    }
    
    route->points[route->pointCount].x = x;
    route->points[route->pointCount].z = z;
    route->points[route->pointCount].width = width;
    route->pointCount++;
}

// Interpolação linear entre dois pontos
static void Route_InterpolateLinear(const Route* route, float t, float* outX, float* outZ, float* outWidth) {
    if (route->pointCount < 2) {
        if (route->pointCount == 1) {
            *outX = route->points[0].x;
            *outZ = route->points[0].z;
            *outWidth = route->points[0].width;
        }
        return;
    }
    
    float segmentLength = 1.0f / (route->pointCount - 1);
    int segment = (int)(t / segmentLength);
    if (segment >= route->pointCount - 1) segment = route->pointCount - 2;
    
    float localT = (t - segment * segmentLength) / segmentLength;
    if (localT < 0.0f) localT = 0.0f;
    if (localT > 1.0f) localT = 1.0f;
    
    const RoutePoint* p0 = &route->points[segment];
    const RoutePoint* p1 = &route->points[segment + 1];
    
    *outX = p0->x + (p1->x - p0->x) * localT;
    *outZ = p0->z + (p1->z - p0->z) * localT;
    *outWidth = p0->width + (p1->width - p0->width) * localT;
}

void Route_GetPositionAt(const Route* route, float t, float* outX, float* outZ, float* outWidth) {
    if (!route || !outX || !outZ || !outWidth) return;
    
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    
    Route_InterpolateLinear(route, t, outX, outZ, outWidth);
}

float Route_GetClosestT(const Route* route, float x, float z) {
    if (!route || route->pointCount < 2) return 0.0f;
    
    float minDist = INFINITY;
    float bestT = 0.0f;
    
    // Procura no segmento mais próximo
    for (int i = 0; i < route->pointCount - 1; i++) {
        const RoutePoint* p0 = &route->points[i];
        const RoutePoint* p1 = &route->points[i + 1];
        
        // Projeta ponto no segmento
        float dx = p1->x - p0->x;
        float dz = p1->z - p0->z;
        float len2 = dx * dx + dz * dz;
        
        if (len2 < 0.0001f) continue;
        
        float t = ((x - p0->x) * dx + (z - p0->z) * dz) / len2;
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        
        float px = p0->x + dx * t;
        float pz = p0->z + dz * t;
        float dist = (x - px) * (x - px) + (z - pz) * (z - pz);
        
        if (dist < minDist) {
            minDist = dist;
            float segmentT = (float)i / (float)(route->pointCount - 1);
            float localT = t / (float)(route->pointCount - 1);
            bestT = segmentT + localT;
        }
    }
    
    return bestT;
}

bool Route_IsPointOnRoute(const Route* route, float x, float z, float tolerance) {
    if (!route) return false;
    
    float t = Route_GetClosestT(route, x, z);
    float routeX, routeZ, routeWidth;
    Route_GetPositionAt(route, t, &routeX, &routeZ, &routeWidth);
    
    float dist = sqrtf((x - routeX) * (x - routeX) + (z - routeZ) * (z - routeZ));
    return dist <= (routeWidth * 0.5f + tolerance);
}

void Route_Generate(Route* route, VoxelWorld* world, int32_t mapMinX, int32_t mapMaxX,
                   int32_t mapMinZ, int32_t mapMaxZ) {
    if (!route || !world) return;
    
    // Limpa pontos existentes
    route->pointCount = 0;
    
    // Usa seed do mundo para gerar rota determinística
    uint64_t seed = VoxelWorld_GetSeedU64(world);
    RNG_Seed((uint32_t)seed);
    
    // Gera corredor reto de ponta a ponta (beco longo)
    // Rota vai de (mapMinX, centroZ) até (mapMaxX, centroZ)
    float centerZ = (float)(mapMinZ + mapMaxZ) * 0.5f;
    float corridorWidth = 8.0f; // Largura fixa do corredor
    
    // Cria pontos ao longo do eixo X (corredor reto)
    int32_t numPoints = 20; // Mais pontos para corredor longo
    float mapWidth = (float)(mapMaxX - mapMinX);
    
    for (int i = 0; i < numPoints; i++) {
        float t = (float)i / (float)(numPoints - 1);
        float x = mapMinX + mapWidth * t;
        float z = centerZ; // Mantém no centro Z (corredor reto)
        
        Route_AddPoint(route, x, z, corridorWidth);
    }
    
    // Calcula comprimento total
    route->totalLength = 0.0f;
    for (int i = 0; i < route->pointCount - 1; i++) {
        float dx = route->points[i + 1].x - route->points[i].x;
        float dz = route->points[i + 1].z - route->points[i].z;
        route->totalLength += sqrtf(dx * dx + dz * dz);
    }
}

void Route_ApplyToWorld(Route* route, VoxelWorld* world) {
    if (!route || !world) return;
    
    // Cria corredor retangular ao longo da rota
    int32_t samples = (int32_t)(route->totalLength); // 1 amostra por unidade
    if (samples < 50) samples = 50;
    
    int32_t floorY = 63; // Altura do chão
    int32_t wallHeight = 20; // Altura das paredes
    
    for (int i = 0; i <= samples; i++) {
        float t = (float)i / (float)samples;
        float x, z, width;
        Route_GetPositionAt(route, t, &x, &z, &width);
        
        int32_t centerX = (int32_t)x;
        int32_t centerZ = (int32_t)z;
        int32_t halfWidth = (int32_t)(width * 0.5f);
        
        // Cria seção do corredor
        for (int32_t dx = -halfWidth; dx <= halfWidth; dx++) {
            for (int32_t dz = -halfWidth; dz <= halfWidth; dz++) {
                // Chão do corredor (laranja)
                Voxel floorVoxel = {BLOCK_ORANGE, 0};
                VoxelWorld_SetBlock(world, centerX + dx, floorY, centerZ + dz, floorVoxel);
                
                // Paredes laterais do corredor (cinza)
                if (dx == -halfWidth || dx == halfWidth || dz == -halfWidth || dz == halfWidth) {
                    for (int32_t wallY = floorY + 1; wallY <= floorY + wallHeight; wallY++) {
                        Voxel wallVoxel = {BLOCK_GRAY, 0};
                        VoxelWorld_SetBlock(world, centerX + dx, wallY, centerZ + dz, wallVoxel);
                    }
                }
            }
        }
    }
}
