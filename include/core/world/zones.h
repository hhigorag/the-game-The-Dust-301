#ifndef ZONES_H
#define ZONES_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct VoxelWorld VoxelWorld;
struct Route;
typedef struct Route Route;

// Estrutura de uma zona de anomalia
typedef struct {
    float centerX, centerY, centerZ; // Centro da zona
    float radius;                     // Raio da zona
    float intensity;                 // Intensidade (0.0 a 1.0)
    uint32_t id;                      // ID único
    bool active;                      // Zona ativa?
} AnomalyZone;

// Estrutura do sistema de zonas
struct ZoneSystem {
    AnomalyZone* zones;   // Array de zonas
    int32_t count;        // Número de zonas
    int32_t capacity;     // Capacidade do array
    uint32_t nextId;      // Próximo ID disponível
};
typedef struct ZoneSystem ZoneSystem;

// Cria o sistema de zonas
ZoneSystem* ZoneSystem_Create(void);

// Destrói o sistema de zonas
void ZoneSystem_Destroy(ZoneSystem* system);

// Gera zonas de anomalia baseadas na seed
void ZoneSystem_Generate(ZoneSystem* system, VoxelWorld* world, const Route* route,
                        int32_t mapMinX, int32_t mapMaxX,
                        int32_t mapMinZ, int32_t mapMaxZ,
                        int32_t count);

// Adiciona uma zona manualmente
void ZoneSystem_Add(ZoneSystem* system, float x, float y, float z, float radius, float intensity);

// Retorna a intensidade de anomalia em uma posição (soma de todas as zonas)
float ZoneSystem_GetIntensityAt(ZoneSystem* system, float x, float y, float z);

// Verifica se uma posição está dentro de alguma zona
bool ZoneSystem_IsInZone(ZoneSystem* system, float x, float y, float z);

// Aplica zonas ao mundo voxel (pinta centro verde, área roxa)
void ZoneSystem_ApplyToWorld(ZoneSystem* system, VoxelWorld* world);

#endif // ZONES_H
