#include "app/render/voxel_renderer.h"
#include "app/render/voxel_mesh.h"
#include "core/world/voxel_world.h"
#include "core/world/chunk.h"
#include "core/math/core_math.h"
#include <raylib.h>
#include <math.h>
#include <string.h>

// Mesh estático para renderização (reutilizado entre chunks)
static VoxelMesh g_chunkMesh = {0};
static bool g_meshInitialized = false;

static BlockColor g_blockColors[] = {
    {0, 0, 0, 0},           // BLOCK_AIR - transparente
    {20, 20, 20, 255},      // BLOCK_BLACK - preto
    {255, 50, 50, 255},     // BLOCK_RED - vermelho
    {255, 165, 0, 255},     // BLOCK_ORANGE - laranja
    {128, 128, 128, 255},   // BLOCK_GRAY - cinza
    {50, 255, 50, 255},     // BLOCK_GREEN - verde
    {128, 0, 128, 200},     // BLOCK_PURPLE - roxo opaco
    {138, 43, 226, 255},    // BLOCK_VIOLET - violeta
    {101, 67, 33, 255},     // BLOCK_TERRAIN - marrom (terreno)
};

void VoxelRenderer_Init(VoxelRenderer* renderer) {
    if (!renderer) return;
    memset(renderer, 0, sizeof(VoxelRenderer));
    renderer->renderDistance = 4; // 4 chunks de distância
    renderer->initialized = true;
    
    // Inicializa mesh se ainda não foi inicializado
    if (!g_meshInitialized) {
        VoxelMesh_Init(&g_chunkMesh);
        g_meshInitialized = true;
    }
}

BlockColor VoxelRenderer_GetBlockColor(uint8_t blockType) {
    if (blockType >= sizeof(g_blockColors) / sizeof(g_blockColors[0])) {
        BlockColor c = {255, 255, 255, 255};
        return c;
    }
    return g_blockColors[blockType];
}

void VoxelRenderer_SetRenderDistance(VoxelRenderer* renderer, int32_t distance) {
    if (renderer) {
        renderer->renderDistance = distance;
    }
}

void VoxelRenderer_Render(VoxelRenderer* renderer, VoxelWorld* world, 
                         float playerX, float playerY, float playerZ,
                         Camera3D* camera) {
    if (!renderer || !renderer->initialized || !world || !camera) return;
    
    // Inicializa mesh se necessário
    if (!g_meshInitialized) {
        VoxelMesh_Init(&g_chunkMesh);
        g_meshInitialized = true;
    }
    
    // Atualiza streaming de chunks (raio para 30m)
    const float MAX_RENDER_DISTANCE = 30.0f;
    int32_t chunkRadius = (int32_t)ceilf(MAX_RENDER_DISTANCE / (float)CHUNK_SIZE_X) + 1; // Chunks necessários para 30m
    VoxelWorld_UpdateStreaming(world, playerX, playerY, playerZ, chunkRadius);
    
    // Calcula chunks visíveis (corrigido para coordenadas negativas)
    // Usa a mesma lógica do Chunk_GlobalToLocal para garantir consistência
    int32_t playerChunkX = (playerX < 0) ? ((int32_t)playerX + 1) / CHUNK_SIZE_X - 1 : (int32_t)playerX / CHUNK_SIZE_X;
    int32_t playerChunkZ = (playerZ < 0) ? ((int32_t)playerZ + 1) / CHUNK_SIZE_Z - 1 : (int32_t)playerZ / CHUNK_SIZE_Z;
    
    // Limpa mesh antes de gerar novos chunks
    VoxelMesh_Clear(&g_chunkMesh);
    
    // Gera mesh para chunks em raio (renderiza todos os chunks carregados)
    // Aumenta o raio para garantir que o mapa seja renderizado
    int32_t renderRadius = chunkRadius + 2; // Adiciona margem extra
    
    for (int32_t dz = -renderRadius; dz <= renderRadius; dz++) {
        for (int32_t dx = -renderRadius; dx <= renderRadius; dx++) {
            int32_t chunkX = playerChunkX + dx;
            int32_t chunkZ = playerChunkZ + dz;
            
            // Gera mesh do chunk (sem verificação de distância - renderiza todos)
            VoxelMesh_GenerateChunk(&g_chunkMesh, world, 
                                   chunkX, chunkZ,
                                   playerX, playerY, playerZ);
        }
    }
    
    // Renderiza o mesh completo
    VoxelMesh_Render(&g_chunkMesh, camera);
}
