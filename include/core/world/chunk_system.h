#ifndef CHUNK_SYSTEM_H
#define CHUNK_SYSTEM_H

#include "core/math/core_math.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// CHUNK SYSTEM (STUBS DE INTEGRAÇÃO)
// ============================================================================
// Este módulo fornece stubs para integração com o sistema de chunks existente.
// A implementação real deve ser feita no sistema de geração procedural.
// ============================================================================

// Chunk ID (coordenadas do chunk)
typedef struct {
    int32_t x, z; // Coordenadas do chunk (não coordenadas do mundo)
} ChunkID;

// Estado do chunk
typedef enum {
    CHUNK_STATE_EMPTY,
    CHUNK_STATE_GENERATING,
    CHUNK_STATE_READY,
    CHUNK_STATE_INVALID
} ChunkState;

// Informações do chunk para culling
typedef struct {
    ChunkID id;
    Vec3 center;      // Centro do chunk (world space)
    Vec3 min;         // Canto mínimo do AABB (world space)
    Vec3 max;         // Canto máximo do AABB (world space)
    ChunkState state; // Estado do chunk
} ChunkInfo;

// ============================================================================
// API DE INTEGRAÇÃO
// ============================================================================

// Obtém chunks dentro do raio de streaming
// Retorna número de chunks encontrados
// chunks: array de saída (deve ter espaço para pelo menos maxChunks)
// playerPos: posição do player
// radius: raio de streaming (em chunks)
int ChunkSystem_GetChunksInRadius(ChunkInfo* chunks, int maxChunks, Vec3 playerPos, int radius);

// Verifica se chunk está pronto para renderização
bool ChunkSystem_IsChunkReady(const ChunkInfo* chunk);

// Obtém AABB do chunk (para culling)
void ChunkSystem_GetChunkAABB(const ChunkInfo* chunk, Vec3* outMin, Vec3* outMax);

#endif // CHUNK_SYSTEM_H
