#ifndef ROUTE_H
#define ROUTE_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct VoxelWorld VoxelWorld;

// Ponto de controle da rota (spline)
typedef struct {
    float x, z;             // Posição 2D (X, Z)
    float width;            // Largura da rota neste ponto
} RoutePoint;

// Estrutura da rota principal
struct Route {
    RoutePoint* points;     // Array de pontos de controle
    int32_t pointCount;     // Número de pontos
    int32_t capacity;       // Capacidade do array
    float totalLength;      // Comprimento total da rota
    bool initialized;
};
typedef struct Route Route;

// Cria uma rota vazia
Route* Route_Create(void);

// Destrói uma rota
void Route_Destroy(Route* route);

// Gera a rota principal baseada na seed
// A rota atravessa o mapa de ponta a ponta
void Route_Generate(Route* route, VoxelWorld* world, int32_t mapMinX, int32_t mapMaxX,
                   int32_t mapMinZ, int32_t mapMaxZ);

// Adiciona um ponto à rota
void Route_AddPoint(Route* route, float x, float z, float width);

// Retorna a posição na rota em uma distância t (0.0 a 1.0)
void Route_GetPositionAt(const Route* route, float t, float* outX, float* outZ, float* outWidth);

// Retorna a posição mais próxima na rota de um ponto (x, z)
float Route_GetClosestT(const Route* route, float x, float z);

// Verifica se um ponto (x, z) está dentro da rota (considerando largura)
bool Route_IsPointOnRoute(const Route* route, float x, float z, float tolerance);

// Aplica a rota ao mundo voxel (pinta blocos laranja)
void Route_ApplyToWorld(Route* route, VoxelWorld* world);

#endif // ROUTE_H
