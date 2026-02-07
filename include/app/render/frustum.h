#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "core/math/core_math.h"
#include <stdbool.h>

// ============================================================================
// FRUSTUM CULLING
// ============================================================================
// Sistema de frustum culling para otimizar renderização.
// Frustum é calculado a partir da câmera (pos, forward, up, fov, aspect, near, far).
// Far é clampado a 15m + margem anti-pop.
// ============================================================================

// Plano: ax + by + cz + d = 0
typedef struct {
    Vec3 normal;  // Normal do plano (normalizado)
    float d;      // Distância da origem ao plano
} Plane;

// Frustum: 6 planos (near, far, left, right, top, bottom)
typedef struct {
    Plane planes[6];
    Vec3 camPos;      // Posição da câmera (para cálculos de distância)
    float farDist;    // Distância far efetiva (15m + margem)
} Frustum;

// ============================================================================
// CÁLCULO DO FRUSTUM
// ============================================================================

// Calcula frustum a partir da câmera
// camPos: posição da câmera
// camForward: direção forward da câmera (normalizado)
// camUp: direção up da câmera (normalizado)
// fovY: campo de visão vertical (radianos)
// aspect: aspect ratio (width/height)
// near: distância do plano near
// far: distância do plano far (será clampado a 15m)
void Frustum_Calculate(Frustum* frustum,
                       Vec3 camPos, Vec3 camForward, Vec3 camUp,
                       float fovY, float aspect, float near, float far);

// ============================================================================
// TESTES DE CULLING
// ============================================================================

// Testa se um ponto está dentro do frustum
bool Frustum_IsPointInside(const Frustum* frustum, Vec3 point);

// Testa se uma esfera está dentro do frustum
bool Frustum_IsSphereInside(const Frustum* frustum, Vec3 center, float radius);

// Testa se um AABB está dentro do frustum
// min/max: cantos opostos do AABB
bool Frustum_IsAABBInside(const Frustum* frustum, Vec3 min, Vec3 max);

// Testa distância ao quadrado (evita sqrt)
// Retorna distância² do ponto ao centro da câmera
float Frustum_DistSqToCamera(const Frustum* frustum, Vec3 point);

// ============================================================================
// CULLING COMBINADO (distância + frustum)
// ============================================================================

// Testa chunk: distância + frustum
// Retorna true se chunk deve ser renderizado
bool Frustum_ShouldRenderChunk(const Frustum* frustum, Vec3 chunkCenter, Vec3 chunkMin, Vec3 chunkMax);

#endif // FRUSTUM_H
