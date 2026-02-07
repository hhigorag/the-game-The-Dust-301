#include "core/world/chunk_system.h"

// ============================================================================
// STUB IMPLEMENTATION
// ============================================================================
// Esta é uma implementação stub. A implementação real deve:
// 1. Gerenciar chunks gerados proceduralmente
// 2. Fazer streaming baseado na posição do player
// 3. Retornar chunks reais do sistema de geração
// ============================================================================

#define CHUNK_SIZE 16
#define CHUNK_HEIGHT 256

int ChunkSystem_GetChunksInRadius(ChunkInfo* chunks, int maxChunks, Vec3 playerPos, int radius) {
    if (!chunks || maxChunks <= 0) return 0;
    
    // STUB: Retorna chunks vazios para teste
    // Implementação real deve:
    // 1. Calcular chunk do player: chunkX = (int)floor(playerPos.x / CHUNK_SIZE)
    // 2. Iterar sobre chunks no raio
    // 3. Gerar/carregar chunks que não existem
    // 4. Retornar chunks prontos
    
    int count = 0;
    for (int dz = -radius; dz <= radius && count < maxChunks; dz++) {
        for (int dx = -radius; dx <= radius && count < maxChunks; dx++) {
            ChunkID id = {dx, dz};
            
            // Calcula centro do chunk (world space)
            float chunkWorldX = (float)(dx * CHUNK_SIZE) + (CHUNK_SIZE * 0.5f);
            float chunkWorldZ = (float)(dz * CHUNK_SIZE) + (CHUNK_SIZE * 0.5f);
            
            chunks[count].id = id;
            chunks[count].center = Vec3_Make(chunkWorldX, CHUNK_HEIGHT * 0.5f, chunkWorldZ);
            chunks[count].min = Vec3_Make(
                (float)(dx * CHUNK_SIZE),
                0.0f,
                (float)(dz * CHUNK_SIZE)
            );
            chunks[count].max = Vec3_Make(
                (float)((dx + 1) * CHUNK_SIZE),
                (float)CHUNK_HEIGHT,
                (float)((dz + 1) * CHUNK_SIZE)
            );
            chunks[count].state = CHUNK_STATE_READY;
            
            count++;
        }
    }
    
    return count;
}

bool ChunkSystem_IsChunkReady(const ChunkInfo* chunk) {
    if (!chunk) return false;
    return chunk->state == CHUNK_STATE_READY;
}

void ChunkSystem_GetChunkAABB(const ChunkInfo* chunk, Vec3* outMin, Vec3* outMax) {
    if (!chunk || !outMin || !outMax) return;
    *outMin = chunk->min;
    *outMax = chunk->max;
}
