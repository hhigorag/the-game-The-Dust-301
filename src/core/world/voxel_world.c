#include "core/world/voxel_world.h"
#include "core/world/chunk.h"
#include "core/world/world_seed.h"
#include "core/world/segment_manager.h"
#include "core/world/event_system.h"
#include "core/world/structure_spawner.h"
#include "core/world/world_config.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define HASH_TABLE_SIZE 256

// Hash table de chunks
typedef struct {
    Chunk* buckets[HASH_TABLE_SIZE];
} ChunkHashTable;

// Estrutura do mundo voxel
struct VoxelWorld {
    char seedString[256];   // Seed como string
    uint64_t globalSeed;    // Seed como uint64
    ChunkHashTable chunks;  // Hash table de chunks
    int32_t loadedChunkCount;
    int32_t generatingChunkCount;
};

static uint32_t ChunkHash_GetHash(int32_t chunkX, int32_t chunkZ) {
    // Hash simples baseado em coordenadas
    uint32_t h = (uint32_t)(chunkX * 73856093) ^ (uint32_t)(chunkZ * 19349663);
    return h % HASH_TABLE_SIZE;
}

static Chunk* ChunkHash_Find(ChunkHashTable* table, int32_t chunkX, int32_t chunkZ) {
    uint32_t hash = ChunkHash_GetHash(chunkX, chunkZ);
    Chunk* chunk = table->buckets[hash];
    
    while (chunk) {
        if (chunk->chunkX == chunkX && chunk->chunkZ == chunkZ) {
            return chunk;
        }
        chunk = chunk->next;
    }
    
    return NULL;
}

static void ChunkHash_Insert(ChunkHashTable* table, Chunk* chunk) {
    uint32_t hash = ChunkHash_GetHash(chunk->chunkX, chunk->chunkZ);
    chunk->next = table->buckets[hash];
    table->buckets[hash] = chunk;
}

static void ChunkHash_Remove(ChunkHashTable* table, int32_t chunkX, int32_t chunkZ) {
    uint32_t hash = ChunkHash_GetHash(chunkX, chunkZ);
    Chunk* chunk = table->buckets[hash];
    Chunk* prev = NULL;
    
    while (chunk) {
        if (chunk->chunkX == chunkX && chunk->chunkZ == chunkZ) {
            if (prev) {
                prev->next = chunk->next;
            } else {
                table->buckets[hash] = chunk->next;
            }
            Chunk_Destroy(chunk);
            return;
        }
        prev = chunk;
        chunk = chunk->next;
    }
}

VoxelWorld* VoxelWorld_Create(const char* seedString) {
    VoxelWorld* world = (VoxelWorld*)calloc(1, sizeof(VoxelWorld));
    if (!world) return NULL;
    
    if (seedString) {
        strncpy(world->seedString, seedString, sizeof(world->seedString) - 1);
        world->globalSeed = WorldSeed_StringToU64(seedString);
    } else {
        strcpy(world->seedString, "VOID-EXPLORER-001");
        world->globalSeed = WorldSeed_StringToU64(world->seedString);
    }
    
    memset(&world->chunks, 0, sizeof(world->chunks));
    world->loadedChunkCount = 0;
    world->generatingChunkCount = 0;
    
    return world;
}

void VoxelWorld_Destroy(VoxelWorld* world) {
    if (!world) return;
    
    // Destrói todos os chunks
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        Chunk* chunk = world->chunks.buckets[i];
        while (chunk) {
            Chunk* next = chunk->next;
            Chunk_Destroy(chunk);
            chunk = next;
        }
    }
    
    free(world);
}

void VoxelWorld_SetSeed(VoxelWorld* world, const char* seedString) {
    if (!world || !seedString) return;
    
    strncpy(world->seedString, seedString, sizeof(world->seedString) - 1);
    world->globalSeed = WorldSeed_StringToU64(seedString);
    
    // Limpa chunks existentes (seed mudou)
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        Chunk* chunk = world->chunks.buckets[i];
        while (chunk) {
            Chunk* next = chunk->next;
            Chunk_Destroy(chunk);
            chunk = next;
        }
        world->chunks.buckets[i] = NULL;
    }
    world->loadedChunkCount = 0;
    world->generatingChunkCount = 0;
}

uint64_t VoxelWorld_GetSeedU64(VoxelWorld* world) {
    return world ? world->globalSeed : 0;
}

Chunk* VoxelWorld_GetChunk(VoxelWorld* world, int32_t chunkX, int32_t chunkZ) {
    if (!world) return NULL;
    
    // Procura chunk existente
    Chunk* chunk = ChunkHash_Find(&world->chunks, chunkX, chunkZ);
    if (chunk) return chunk;
    
    // Cria novo chunk
    uint64_t chunkSeed = WorldSeed_GetChunkSeed(world->globalSeed, chunkX, chunkZ);
    chunk = Chunk_Create(chunkX, chunkZ, chunkSeed);
    if (!chunk) return NULL;
    
    ChunkHash_Insert(&world->chunks, chunk);
    world->loadedChunkCount++;
    world->generatingChunkCount++;
    
    // Marca como pronto (geração será feita depois)
    chunk->state = CHUNK_STATE_READY;
    world->generatingChunkCount--;
    
    return chunk;
}

void VoxelWorld_UnloadChunk(VoxelWorld* world, int32_t chunkX, int32_t chunkZ) {
    if (!world) return;
    
    Chunk* chunk = ChunkHash_Find(&world->chunks, chunkX, chunkZ);
    if (chunk) {
        ChunkHash_Remove(&world->chunks, chunkX, chunkZ);
        world->loadedChunkCount--;
    }
}

Voxel VoxelWorld_GetBlock(VoxelWorld* world, int32_t x, int32_t y, int32_t z) {
    if (!world) {
        Voxel air = {BLOCK_AIR, 0};
        return air;
    }
    
    int32_t chunkX, chunkZ, localX, localY, localZ;
    Chunk_GlobalToLocal(x, y, z, &chunkX, &chunkZ, &localX, &localY, &localZ);
    
    Chunk* chunk = ChunkHash_Find(&world->chunks, chunkX, chunkZ);
    if (!chunk) {
        Voxel air = {BLOCK_AIR, 0};
        return air;
    }
    
    return Chunk_GetBlock(chunk, localX, localY, localZ);
}

void VoxelWorld_SetBlock(VoxelWorld* world, int32_t x, int32_t y, int32_t z, Voxel voxel) {
    if (!world) return;
    
    int32_t chunkX, chunkZ, localX, localY, localZ;
    Chunk_GlobalToLocal(x, y, z, &chunkX, &chunkZ, &localX, &localY, &localZ);
    
    Chunk* chunk = VoxelWorld_GetChunk(world, chunkX, chunkZ);
    if (chunk) {
        Chunk_SetBlock(chunk, localX, localY, localZ, voxel);
    }
}

void VoxelWorld_UpdateStreaming(VoxelWorld* world, float playerX, float playerY, float playerZ, int32_t loadRadius) {
    (void)playerY; // Usado no futuro para altura
    if (!world) return;
    
    // Converte posição do player para coordenadas de chunk (corrigido para negativos)
    int32_t playerChunkX = (playerX < 0) ? ((int32_t)playerX + 1) / CHUNK_SIZE_X - 1 : (int32_t)playerX / CHUNK_SIZE_X;
    int32_t playerChunkZ = (playerZ < 0) ? ((int32_t)playerZ + 1) / CHUNK_SIZE_Z - 1 : (int32_t)playerZ / CHUNK_SIZE_Z;
    
    // Carrega chunks em raio
    for (int32_t dz = -loadRadius; dz <= loadRadius; dz++) {
        for (int32_t dx = -loadRadius; dx <= loadRadius; dx++) {
            int32_t chunkX = playerChunkX + dx;
            int32_t chunkZ = playerChunkZ + dz;
            
            // Verifica se está dentro do raio
            if (dx * dx + dz * dz <= loadRadius * loadRadius) {
                VoxelWorld_GetChunk(world, chunkX, chunkZ);
            }
        }
    }
    
    // TODO: Descarregar chunks fora do raio (otimização futura)
}

/* Largura do corredor carregável: 20 voxel chunks de cada lado do centro. */
#define STREAM_CORRIDOR_HALF_VOXEL_CHUNKS 20

/* Mundo X = [-500..+500] m. Hash chunk X -32..31; borda esquerda = -32.
 * centerVX_0_62 = floor((centerX_m + 500) / 16). centerChunkX = centerVX_0_62 - 31 (centro = 0 no hash). */
#define WORLD_X_LEFT_M           (-500.0f)
#define WORLD_X_CENTER_CHUNK_OFFSET  31  /* em 0..62 o centro é 31; hash centro = 0 */
#define WORLD_X_CHUNK_TO_METER_OFFSET 32 /* (chunkX + 32)*16 - 500 = worldX borda; chunk -32 → -500 */

void VoxelWorld_UpdateStreamingFromRange(VoxelWorld* world, int32_t minMacroZ, int32_t maxMacroZ, float centerX_m, float playerX, float playerZ) {
    if (!world) return;
    
    /* Chunk do player: nunca descarregar (evita cair no vazio). */
    int32_t playerChunkX = 0, playerChunkZ = 0, dummyLocalX, dummyLocalY, dummyLocalZ;
    Chunk_GlobalToLocal((int32_t)playerX, 0, (int32_t)playerZ, &playerChunkX, &playerChunkZ, &dummyLocalX, &dummyLocalY, &dummyLocalZ);
    
    /* StreamingController usa chunks macro 32 m. Voxel chunk = 16 m → 1 macro = 2 voxel. */
    int32_t minVoxelZ = minMacroZ * 2;
    int32_t maxVoxelZ = maxMacroZ * 2 + 1; /* inclusivo; cobre o último macro */
    if (maxVoxelZ > (CHUNKS_LONG * CHUNK_SIZE_M / CHUNK_SIZE_Z) - 1) {
        maxVoxelZ = (CHUNKS_LONG * CHUNK_SIZE_M / CHUNK_SIZE_Z) - 1;
    }
    
    /* Centro do corredor: mundo [-500..+500] → centerVX = floor((centerX_m + 500) / 16); hash -32..31 = centerVX - 31. */
    int32_t centerVX_0_62 = (int)floorf((centerX_m + 500.0f) / (float)CHUNK_SIZE_X);
    if (centerVX_0_62 < 0) centerVX_0_62 = 0;
    if (centerVX_0_62 > 62) centerVX_0_62 = 62;
    int32_t centerChunkX = centerVX_0_62 - WORLD_X_CENTER_CHUNK_OFFSET; /* nosso hash: centro = 0 */
    int32_t minVoxelX = centerChunkX - STREAM_CORRIDOR_HALF_VOXEL_CHUNKS;
    int32_t maxVoxelX = centerChunkX + STREAM_CORRIDOR_HALF_VOXEL_CHUNKS;
    if (minVoxelX < -32) minVoxelX = -32;
    if (maxVoxelX > 31) maxVoxelX = 31;
    
    /* 1) Carregar/gerar chunks na faixa [minVoxelX..maxVoxelX] x [minVoxelZ..maxVoxelZ] */
    for (int32_t vz = minVoxelZ; vz <= maxVoxelZ; vz++) {
        int32_t macroChunkZ = vz / 2;
        float corridorCenterX = SegmentManager_GetCorridorCenterX(world->globalSeed, macroChunkZ);
        SegmentType segType = SegmentManager_GetTypeByChunkZ(macroChunkZ);
        int32_t segmentIndex = SegmentManager_GetSegmentIndex(macroChunkZ);
        SegmentEventType eventType = EventSystem_GetEventForSegment(world->globalSeed, segmentIndex);
        LargeStructureType structType = LARGE_STRUCT_NONE;
        if (StructureSpawner_ShouldSpawnAtChunk(world->globalSeed, macroChunkZ)) {
            structType = StructureSpawner_GetTypeAtChunk(world->globalSeed, macroChunkZ);
        }
        
        for (int32_t vx = minVoxelX; vx <= maxVoxelX; vx++) {
            Chunk* chunk = ChunkHash_Find(&world->chunks, vx, vz);
            if (!chunk) {
                uint64_t chunkSeed = WorldSeed_GetChunkSeed(world->globalSeed, vx, vz);
                chunk = Chunk_Create(vx, vz, chunkSeed);
                if (!chunk) continue;
                ChunkHash_Insert(&world->chunks, chunk);
                world->loadedChunkCount++;
                world->generatingChunkCount++;
                
                ChunkGenContext ctx = {
                    .worldSeed = world->globalSeed,
                    .chunkX = vx,
                    .chunkZ = vz,
                    .segType = segType,
                    .corridorCenterX_m = corridorCenterX,
                    .eventType = eventType,
                    .structType = structType,
                    .threatLevel = 0.0f,
                };
                VoxelWorld_GenerateChunk(world, chunk, &ctx);
                chunk->state = CHUNK_STATE_READY;
                world->generatingChunkCount--;
            }
        }
    }
    
    /* 2) Descarregar chunks fora da faixa (nunca o chunk do player). */
    for (int32_t i = 0; i < HASH_TABLE_SIZE; i++) {
        Chunk* chunk = world->chunks.buckets[i];
        Chunk* prev = NULL;
        while (chunk) {
            Chunk* next = chunk->next;
            int32_t vx = chunk->chunkX;
            int32_t vz = chunk->chunkZ;
            int32_t isPlayerChunk = (vx == playerChunkX && vz == playerChunkZ);
            int32_t outZ = (vz < minVoxelZ || vz > maxVoxelZ);
            int32_t outX = (vx < minVoxelX || vx > maxVoxelX);
            if (!isPlayerChunk && (outZ || outX)) {
                if (prev) prev->next = next;
                else world->chunks.buckets[i] = next;
                Chunk_Destroy(chunk);
                world->loadedChunkCount--;
                chunk = next;
                prev = prev; /* prev unchanged when we remove */
            } else {
                prev = chunk;
                chunk = next;
            }
        }
    }
}

/* Mundo X = [-500..+500] m. vx (chunk -32..31) → worldX = (vx+32)*16 + localX + 0.5 - 500. */
#define WORLD_X_ORIGIN_OFFSET_M  500.0f
#define CORRIDOR_RICH_M          150.0f
#define CORRIDOR_BAD_M           350.0f   /* 150..350 = terreno ruim (cascalho/dunas) */

/* Ruído de borda: irregulariza TERRAIN/GRAY/BLACK para evitar “corte reto”. ±20 m de variação. */
#define BORDER_NOISE_VARIATION_M  40.0f   /* (noise - 0.5) * 40 → ±20 m */
#define BORDER_NOISE_SEED_OFFSET  999ULL

/* Value noise bruto [0,1] para borda (determinístico). */
static float BorderNoiseRaw(uint64_t seed, float t) {
    uint64_t h = WorldSeed_Hash64(&seed, sizeof(seed));
    uint64_t v = WorldSeed_Hash64(&t, sizeof(t));
    h ^= v;
    h *= 0x9e3779b97f4a7c15ULL;
    return (float)((h >> 32) & 0x7FFFFFFF) / (float)0x7FFFFFFF;
}

static float Smoothstep(float x) {
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;
    return x * x * (3.0f - 2.0f * x);
}

/* Ruído suavizado [0,1] para borda orgânica. */
static float BorderNoise(uint64_t seed, float t) {
    float t0 = floorf(t);
    float t1 = t0 + 1.0f;
    float n0 = BorderNoiseRaw(seed, t0);
    float n1 = BorderNoiseRaw(seed, t1);
    float frac = t - t0;
    float s = Smoothstep(frac);
    return n0 + (n1 - n0) * s;
}

void VoxelWorld_GenerateChunk(VoxelWorld* vw, Chunk* c, const ChunkGenContext* ctx) {
    (void)vw;
    if (!c || !ctx) return;
    
    int32_t chunkX = c->chunkX;
    int32_t chunkZ = c->chunkZ;
    float corridorCenterX_m = ctx->corridorCenterX_m;
    uint64_t borderSeed = ctx->worldSeed + BORDER_NOISE_SEED_OFFSET;
    
    for (int32_t localZ = 0; localZ < CHUNK_SIZE_Z; localZ++) {
        for (int32_t localX = 0; localX < CHUNK_SIZE_X; localX++) {
            /* Centro do bloco em mundo X (metros). Mundo [-500..+500]. */
            float worldX = (float)((chunkX + WORLD_X_CHUNK_TO_METER_OFFSET) * CHUNK_SIZE_X + localX) + 0.5f - WORLD_X_ORIGIN_OFFSET_M;
            float dx = fabsf(worldX - corridorCenterX_m);
            
            /* Ruído leve para borda orgânica (evita “corte reto” TERRAIN/GRAY/BLACK). */
            float tNoise = (float)chunkZ * 0.3f + (float)localX * 0.1f;
            float borderNoise = BorderNoise(borderSeed, tNoise);
            float variation = (borderNoise - 0.5f) * BORDER_NOISE_VARIATION_M;  /* ±20 m */
            float effectiveDx = dx + variation;
            
            BlockType floorType = BLOCK_TERRAIN;
            if (effectiveDx > CORRIDOR_BAD_M)
                floorType = BLOCK_BLACK;
            else if (effectiveDx > CORRIDOR_RICH_M)
                floorType = BLOCK_GRAY;  /* 150..350 m: terreno ruim; navegável */
            
            Voxel floor = { floorType, 0 };
            Chunk_SetBlock(c, localX, 0, localZ, floor);
        }
    }
}

void VoxelWorld_GetStats(VoxelWorld* world, int32_t* loadedChunks, int32_t* generatingChunks) {
    if (!world) {
        if (loadedChunks) *loadedChunks = 0;
        if (generatingChunks) *generatingChunks = 0;
        return;
    }
    
    if (loadedChunks) *loadedChunks = world->loadedChunkCount;
    if (generatingChunks) *generatingChunks = world->generatingChunkCount;
}
