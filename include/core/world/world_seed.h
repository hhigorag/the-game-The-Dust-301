#ifndef WORLD_SEED_H
#define WORLD_SEED_H

#include <stdint.h>

// Converte uma string de seed em uint64 (hash determinístico)
uint64_t WorldSeed_StringToU64(const char* seedString);

// Gera a seed de um chunk específico
// seed_chunk = hash(seed_global + chunkX + chunkZ)
uint64_t WorldSeed_GetChunkSeed(uint64_t globalSeed, int32_t chunkX, int32_t chunkZ);

// Hash simples mas eficiente (FNV-1a)
uint64_t WorldSeed_Hash64(const void* data, size_t length);

#endif // WORLD_SEED_H
