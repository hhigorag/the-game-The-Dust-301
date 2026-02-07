#ifndef VOXEL_RENDERER_H
#define VOXEL_RENDERER_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct VoxelWorld VoxelWorld;
typedef struct Camera3D Camera3D;

// Cores para cada tipo de bloco
typedef struct {
    unsigned char r, g, b, a;
} BlockColor;

// Renderizador de mundo voxel
typedef struct {
    bool initialized;
    int32_t renderDistance; // Distância de renderização em chunks
} VoxelRenderer;

// Inicializa o renderizador
void VoxelRenderer_Init(VoxelRenderer* renderer);

// Renderiza o mundo voxel
void VoxelRenderer_Render(VoxelRenderer* renderer, VoxelWorld* world, 
                         float playerX, float playerY, float playerZ,
                         Camera3D* camera);

// Retorna a cor de um tipo de bloco
BlockColor VoxelRenderer_GetBlockColor(uint8_t blockType);

// Define a distância de renderização
void VoxelRenderer_SetRenderDistance(VoxelRenderer* renderer, int32_t distance);

#endif // VOXEL_RENDERER_H
