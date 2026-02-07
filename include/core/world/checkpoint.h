#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct VoxelWorld VoxelWorld;
struct Route;
typedef struct Route Route;

// Estrutura de um checkpoint
typedef struct {
    float x, y, z;          // Posição 3D
    float radius;           // Raio de ativação
    bool activated;         // Já foi ativado?
    uint32_t id;            // ID único
} Checkpoint;

// Estrutura do sistema de checkpoints
struct CheckpointSystem {
    Checkpoint* checkpoints; // Array de checkpoints
    int32_t count;          // Número de checkpoints
    int32_t capacity;       // Capacidade do array
    uint32_t nextId;        // Próximo ID disponível
};
typedef struct CheckpointSystem CheckpointSystem;

// Cria o sistema de checkpoints
CheckpointSystem* CheckpointSystem_Create(void);

// Destrói o sistema de checkpoints
void CheckpointSystem_Destroy(CheckpointSystem* system);

// Gera checkpoints ao longo da rota
void CheckpointSystem_GenerateAlongRoute(CheckpointSystem* system, const Route* route,
                                       VoxelWorld* world, float minDistance);

// Adiciona um checkpoint manualmente
void CheckpointSystem_Add(CheckpointSystem* system, float x, float y, float z, float radius);

// Verifica se um checkpoint foi ativado (player próximo)
Checkpoint* CheckpointSystem_CheckActivation(CheckpointSystem* system, float playerX, float playerY, float playerZ);

// Aplica checkpoints ao mundo voxel (pinta blocos violeta)
void CheckpointSystem_ApplyToWorld(CheckpointSystem* system, VoxelWorld* world);

// Retorna o checkpoint mais próximo de uma posição
Checkpoint* CheckpointSystem_GetNearest(CheckpointSystem* system, float x, float y, float z);

#endif // CHECKPOINT_H
