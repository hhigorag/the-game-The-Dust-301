#include "core/world/voxel_world.h"
#include "core/world/chunk.h"
#include "core/world/world_seed.h"
#include <stdlib.h>
#include <string.h>

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

void VoxelWorld_GetStats(VoxelWorld* world, int32_t* loadedChunks, int32_t* generatingChunks) {
    if (!world) {
        if (loadedChunks) *loadedChunks = 0;
        if (generatingChunks) *generatingChunks = 0;
        return;
    }
    
    if (loadedChunks) *loadedChunks = world->loadedChunkCount;
    if (generatingChunks) *generatingChunks = world->generatingChunkCount;
}
