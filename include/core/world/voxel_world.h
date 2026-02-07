#ifndef VOXEL_WORLD_H
#define VOXEL_WORLD_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct Chunk Chunk;
typedef struct VoxelWorld VoxelWorld;

// Tipos de bloco (cores)
typedef enum {
    BLOCK_AIR = 0,          // Vazio
    BLOCK_BLACK = 1,        // Preto - Borda/limite
    BLOCK_RED = 2,          // Vermelho - Ponto inicial
    BLOCK_ORANGE = 3,       // Laranja - Rota principal
    BLOCK_GRAY = 4,         // Cinza - Construções
    BLOCK_GREEN = 5,        // Verde - Centro de anomalia
    BLOCK_PURPLE = 6,       // Roxo - Área de anomalia
    BLOCK_VIOLET = 7,       // Violeta - Checkpoint
    BLOCK_TERRAIN = 8,      // Terreno base
    BLOCK_COUNT
} BlockType;

// Estrutura de um bloco voxel
typedef struct {
    BlockType type;
    uint8_t metadata;       // Dados extras (ex: variação de textura)
} Voxel;

// Inicializa o sistema de mundo voxel
VoxelWorld* VoxelWorld_Create(const char* seedString);

// Destrói o mundo voxel
void VoxelWorld_Destroy(VoxelWorld* world);

// Define a seed global (string)
void VoxelWorld_SetSeed(VoxelWorld* world, const char* seedString);

// Retorna a seed global como uint64
uint64_t VoxelWorld_GetSeedU64(VoxelWorld* world);

// Retorna o chunk em coordenadas (chunkX, chunkZ)
// Cria o chunk se não existir (streaming)
Chunk* VoxelWorld_GetChunk(VoxelWorld* world, int32_t chunkX, int32_t chunkZ);

// Descarrega um chunk (libera memória)
void VoxelWorld_UnloadChunk(VoxelWorld* world, int32_t chunkX, int32_t chunkZ);

// Retorna o bloco em coordenadas globais (x, y, z)
Voxel VoxelWorld_GetBlock(VoxelWorld* world, int32_t x, int32_t y, int32_t z);

// Define o bloco em coordenadas globais (x, y, z)
void VoxelWorld_SetBlock(VoxelWorld* world, int32_t x, int32_t y, int32_t z, Voxel voxel);

// Atualiza streaming de chunks ao redor de uma posição
void VoxelWorld_UpdateStreaming(VoxelWorld* world, float playerX, float playerY, float playerZ, int32_t loadRadius);

// Retorna estatísticas do mundo
void VoxelWorld_GetStats(VoxelWorld* world, int32_t* loadedChunks, int32_t* generatingChunks);

#endif // VOXEL_WORLD_H
