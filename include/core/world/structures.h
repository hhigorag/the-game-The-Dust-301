#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct VoxelWorld VoxelWorld;
struct Route;
typedef struct Route Route;

// Tipos de estruturas
typedef enum {
    STRUCTURE_BUILDING,     // Prédio
    STRUCTURE_HANGAR,       // Hangar
    STRUCTURE_TOWER,        // Torre
    STRUCTURE_RUIN,         // Ruína industrial
    STRUCTURE_COUNT
} StructureType;

// Estrutura de uma construção
typedef struct {
    float x, y, z;          // Posição (canto inferior esquerdo)
    int32_t width, height, depth; // Dimensões
    StructureType type;     // Tipo de estrutura
    uint32_t id;            // ID único
} Structure;

// Estrutura do sistema de construções
struct StructureSystem {
    Structure* structures;  // Array de construções
    int32_t count;          // Número de construções
    int32_t capacity;       // Capacidade do array
    uint32_t nextId;        // Próximo ID disponível
};
typedef struct StructureSystem StructureSystem;

// Cria o sistema de construções
StructureSystem* StructureSystem_Create(void);

// Destrói o sistema de construções
void StructureSystem_Destroy(StructureSystem* system);

// Gera construções baseadas na seed e rota
void StructureSystem_Generate(StructureSystem* system, VoxelWorld* world, const Route* route,
                             int32_t mapMinX, int32_t mapMaxX,
                             int32_t mapMinZ, int32_t mapMaxZ,
                             int32_t count);

// Adiciona uma construção manualmente
void StructureSystem_Add(StructureSystem* system, float x, float y, float z,
                        int32_t width, int32_t height, int32_t depth, StructureType type);

// Aplica construções ao mundo voxel (pinta blocos cinza)
void StructureSystem_ApplyToWorld(StructureSystem* system, VoxelWorld* world);

// Verifica se uma posição colide com alguma estrutura
bool StructureSystem_CheckCollision(StructureSystem* system, float x, float y, float z);

#endif // STRUCTURES_H
