#include "core/world/world_seed.h"
#include <string.h>

// FNV-1a hash para 64 bits
static const uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
static const uint64_t FNV_PRIME = 1099511628211ULL;

uint64_t WorldSeed_Hash64(const void* data, size_t length) {
    uint64_t hash = FNV_OFFSET_BASIS;
    const uint8_t* bytes = (const uint8_t*)data;
    
    for (size_t i = 0; i < length; i++) {
        hash ^= bytes[i];
        hash *= FNV_PRIME;
    }
    
    return hash;
}

uint64_t WorldSeed_StringToU64(const char* seedString) {
    if (!seedString) return 0;
    size_t len = strlen(seedString);
    return WorldSeed_Hash64(seedString, len);
}

uint64_t WorldSeed_GetChunkSeed(uint64_t globalSeed, int32_t chunkX, int32_t chunkZ) {
    // Combina global seed + coordenadas do chunk
    uint64_t data[3] = {globalSeed, (uint64_t)chunkX, (uint64_t)chunkZ};
    return WorldSeed_Hash64(data, sizeof(data));
}
