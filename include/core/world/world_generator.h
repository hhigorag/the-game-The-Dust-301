#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct VoxelWorld VoxelWorld;
typedef struct Route Route;
typedef struct CheckpointSystem CheckpointSystem;
typedef struct ZoneSystem ZoneSystem;
typedef struct StructureSystem StructureSystem;

// Configuração de geração do mundo
typedef struct {
    int32_t mapMinX, mapMaxX;  // Limites do mapa (X)
    int32_t mapMinZ, mapMaxZ;  // Limites do mapa (Z)
    int32_t mapHeight;         // Altura do mapa (Y)
    float spawnX, spawnY, spawnZ; // Ponto inicial (vermelho)
    int32_t numZones;          // Número de zonas de anomalia
    int32_t numStructures;    // Número de construções
    float checkpointMinDist;  // Distância mínima entre checkpoints
} WorldGenConfig;

// Sistema de geração de mundo
typedef struct {
    VoxelWorld* world;
    Route* route;
    CheckpointSystem* checkpoints;
    ZoneSystem* zones;
    StructureSystem* structures;
    WorldGenConfig config;
    bool initialized;
} WorldGenerator;

// Cria o gerador de mundo
WorldGenerator* WorldGenerator_Create(const char* seedString);

// Destrói o gerador de mundo
void WorldGenerator_Destroy(WorldGenerator* gen);

// Configura os parâmetros de geração
void WorldGenerator_SetConfig(WorldGenerator* gen, const WorldGenConfig* config);

// Gera o mundo completo seguindo o pipeline:
// 1) Limite do mapa (preto)
// 2) Ponto inicial (vermelho)
// 3) Rota principal (laranja)
// 4) Checkpoints (violeta)
// 5) Zonas de anomalia (verde + roxo)
// 6) Construções (cinza)
// 7) Terreno base
void WorldGenerator_Generate(WorldGenerator* gen);

// Retorna o mundo voxel
VoxelWorld* WorldGenerator_GetWorld(WorldGenerator* gen);

// Retorna a rota principal
Route* WorldGenerator_GetRoute(WorldGenerator* gen);

// Retorna o sistema de checkpoints
CheckpointSystem* WorldGenerator_GetCheckpoints(WorldGenerator* gen);

// Retorna o sistema de zonas
ZoneSystem* WorldGenerator_GetZones(WorldGenerator* gen);

// Retorna o sistema de construções
StructureSystem* WorldGenerator_GetStructures(WorldGenerator* gen);

/* Retorna o spawn atual (definido pela geração). */
void WorldGenerator_GetSpawn(const WorldGenerator* gen, float* outX, float* outY, float* outZ);

#endif // WORLD_GENERATOR_H
