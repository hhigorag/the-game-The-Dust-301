#include "core/world/chunk.h"
#include <stdlib.h>
#include <string.h>

Chunk* Chunk_Create(int32_t chunkX, int32_t chunkZ, uint64_t chunkSeed) {
    Chunk* chunk = (Chunk*)calloc(1, sizeof(Chunk));
    if (!chunk) return NULL;
    
    chunk->chunkX = chunkX;
    chunk->chunkZ = chunkZ;
    chunk->chunkSeed = chunkSeed;
    chunk->state = CHUNK_STATE_GENERATING;
    chunk->dirty = false;
    chunk->next = NULL;
    
    // Inicializa todos os blocos como ar
    for (int i = 0; i < CHUNK_VOLUME; i++) {
        chunk->blocks[i].type = BLOCK_AIR;
        chunk->blocks[i].metadata = 0;
    }
    
    return chunk;
}

void Chunk_Destroy(Chunk* chunk) {
    if (chunk) {
        free(chunk);
    }
}

static inline int32_t Chunk_GetIndex(int32_t localX, int32_t localY, int32_t localZ) {
    // Indexação: [y][z][x] = y * (CHUNK_SIZE_Z * CHUNK_SIZE_X) + z * CHUNK_SIZE_X + x
    return localY * (CHUNK_SIZE_Z * CHUNK_SIZE_X) + localZ * CHUNK_SIZE_X + localX;
}

Voxel Chunk_GetBlock(const Chunk* chunk, int32_t localX, int32_t localY, int32_t localZ) {
    if (!chunk || !Chunk_IsValidLocalPos(localX, localY, localZ)) {
        Voxel air = {BLOCK_AIR, 0};
        return air;
    }
    
    int32_t index = Chunk_GetIndex(localX, localY, localZ);
    return chunk->blocks[index];
}

void Chunk_SetBlock(Chunk* chunk, int32_t localX, int32_t localY, int32_t localZ, Voxel voxel) {
    if (!chunk || !Chunk_IsValidLocalPos(localX, localY, localZ)) return;
    
    int32_t index = Chunk_GetIndex(localX, localY, localZ);
    chunk->blocks[index] = voxel;
    chunk->dirty = true;
}

void Chunk_GlobalToLocal(int32_t globalX, int32_t globalY, int32_t globalZ,
                        int32_t* outChunkX, int32_t* outChunkZ,
                        int32_t* outLocalX, int32_t* outLocalY, int32_t* outLocalZ) {
    // Chunk X: floor(globalX / CHUNK_SIZE_X)
    *outChunkX = (globalX < 0) ? ((globalX + 1) / CHUNK_SIZE_X - 1) : (globalX / CHUNK_SIZE_X);
    *outChunkZ = (globalZ < 0) ? ((globalZ + 1) / CHUNK_SIZE_Z - 1) : (globalZ / CHUNK_SIZE_Z);
    
    // Local: módulo
    *outLocalX = ((globalX % CHUNK_SIZE_X) + CHUNK_SIZE_X) % CHUNK_SIZE_X;
    *outLocalZ = ((globalZ % CHUNK_SIZE_Z) + CHUNK_SIZE_Z) % CHUNK_SIZE_Z;
    *outLocalY = globalY; // Y não é chunked
}

void Chunk_LocalToGlobal(int32_t chunkX, int32_t chunkZ,
                        int32_t localX, int32_t localY, int32_t localZ,
                        int32_t* outGlobalX, int32_t* outGlobalY, int32_t* outGlobalZ) {
    *outGlobalX = chunkX * CHUNK_SIZE_X + localX;
    *outGlobalZ = chunkZ * CHUNK_SIZE_Z + localZ;
    *outGlobalY = localY;
}

bool Chunk_IsValidLocalPos(int32_t localX, int32_t localY, int32_t localZ) {
    return localX >= 0 && localX < CHUNK_SIZE_X &&
           localY >= 0 && localY < CHUNK_SIZE_Y &&
           localZ >= 0 && localZ < CHUNK_SIZE_Z;
}
