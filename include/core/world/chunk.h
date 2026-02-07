#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>
#include <stdbool.h>
#include "voxel_world.h"

// Tamanho do chunk: 16x16x256 (X, Z, Y)
#define CHUNK_SIZE_X 16
#define CHUNK_SIZE_Z 16
#define CHUNK_SIZE_Y 256
#define CHUNK_VOLUME (CHUNK_SIZE_X * CHUNK_SIZE_Z * CHUNK_SIZE_Y)

// Estados do chunk
typedef enum {
    CHUNK_STATE_EMPTY,      // Chunk não existe
    CHUNK_STATE_GENERATING, // Chunk em geração
    CHUNK_STATE_READY,      // Chunk pronto
    CHUNK_STATE_INVALID     // Chunk inválido (erro)
} ChunkState;

// Estrutura de um chunk
typedef struct Chunk {
    int32_t chunkX;         // Coordenada X do chunk
    int32_t chunkZ;         // Coordenada Z do chunk
    uint64_t chunkSeed;     // Seed específica deste chunk
    ChunkState state;       // Estado atual
    Voxel blocks[CHUNK_VOLUME]; // Array de blocos (indexado como [y][z][x])
    bool dirty;             // Precisa ser salvo/atualizado
    struct Chunk* next;     // Para hash table
} Chunk;

// Cria um chunk vazio
Chunk* Chunk_Create(int32_t chunkX, int32_t chunkZ, uint64_t chunkSeed);

// Destrói um chunk
void Chunk_Destroy(Chunk* chunk);

// Retorna o bloco em coordenadas locais do chunk (0-15, 0-255, 0-15)
Voxel Chunk_GetBlock(const Chunk* chunk, int32_t localX, int32_t localY, int32_t localZ);

// Define o bloco em coordenadas locais do chunk
void Chunk_SetBlock(Chunk* chunk, int32_t localX, int32_t localY, int32_t localZ, Voxel voxel);

// Converte coordenadas globais para coordenadas locais do chunk
void Chunk_GlobalToLocal(int32_t globalX, int32_t globalY, int32_t globalZ,
                        int32_t* outChunkX, int32_t* outChunkZ,
                        int32_t* outLocalX, int32_t* outLocalY, int32_t* outLocalZ);

// Converte coordenadas locais do chunk para coordenadas globais
void Chunk_LocalToGlobal(int32_t chunkX, int32_t chunkZ,
                        int32_t localX, int32_t localY, int32_t localZ,
                        int32_t* outGlobalX, int32_t* outGlobalY, int32_t* outGlobalZ);

// Verifica se coordenadas locais são válidas
bool Chunk_IsValidLocalPos(int32_t localX, int32_t localY, int32_t localZ);

#endif // CHUNK_H
