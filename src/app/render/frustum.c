#include "app/render/frustum.h"
#include <math.h>

// ============================================================================
// CONSTANTES
// ============================================================================

#define MAX_FAR_DIST 15.0f        // Distância máxima (15 metros)
#define ANTI_POP_MARGIN 2.0f      // Margem anti-pop (metros)
#define EFFECTIVE_FAR (MAX_FAR_DIST + ANTI_POP_MARGIN)

// Índices dos planos
#define PLANE_NEAR 0
#define PLANE_FAR 1
#define PLANE_LEFT 2
#define PLANE_RIGHT 3
#define PLANE_TOP 4
#define PLANE_BOTTOM 5

// ============================================================================
// HELPERS
// ============================================================================

static float Plane_DistanceToPoint(const Plane* plane, Vec3 point) {
    return Vec3_Dot(plane->normal, point) + plane->d;
}

// ============================================================================
// CÁLCULO DO FRUSTUM
// ============================================================================

void Frustum_Calculate(Frustum* frustum,
                      Vec3 camPos, Vec3 camForward, Vec3 camUp,
                      float fovY, float aspect, float near, float far) {
    if (!frustum) return;
    
    // Clamp far a 15m + margem
    if (far > MAX_FAR_DIST) {
        far = MAX_FAR_DIST;
    }
    frustum->farDist = far + ANTI_POP_MARGIN;
    frustum->camPos = camPos;
    
    // Calcula right vector (right-handed)
    Vec3 right = Vec3_Normalize(Vec3_Cross(camForward, camUp));
    
    // Calcula dimensões do frustum no plano near
    float nearHeight = 2.0f * tanf(fovY * 0.5f) * near;
    float nearWidth = nearHeight * aspect;
    
    // Centro do plano near
    Vec3 nearCenter = Vec3_Add(camPos, Vec3_Scale(camForward, near));
    
    // Cantos do plano near
    Vec3 nearTop = Vec3_Add(nearCenter, Vec3_Scale(camUp, nearHeight * 0.5f));
    Vec3 nearBottom = Vec3_Sub(nearCenter, Vec3_Scale(camUp, nearHeight * 0.5f));
    Vec3 nearLeft = Vec3_Sub(nearCenter, Vec3_Scale(right, nearWidth * 0.5f));
    Vec3 nearRight = Vec3_Add(nearCenter, Vec3_Scale(right, nearWidth * 0.5f));
    
    // Centro do plano far
    Vec3 farCenter = Vec3_Add(camPos, Vec3_Scale(camForward, far));
    
    // Dimensões do plano far
    float farHeight = 2.0f * tanf(fovY * 0.5f) * far;
    float farWidth = farHeight * aspect;
    
    // Cantos do plano far
    Vec3 farTop = Vec3_Add(farCenter, Vec3_Scale(camUp, farHeight * 0.5f));
    Vec3 farBottom = Vec3_Sub(farCenter, Vec3_Scale(camUp, farHeight * 0.5f));
    Vec3 farLeft = Vec3_Sub(farCenter, Vec3_Scale(right, farWidth * 0.5f));
    Vec3 farRight = Vec3_Add(farCenter, Vec3_Scale(right, farWidth * 0.5f));
    
    // Calcula planos
    
    // Near plane
    frustum->planes[PLANE_NEAR].normal = camForward;
    frustum->planes[PLANE_NEAR].d = -Vec3_Dot(camForward, nearCenter);
    
    // Far plane
    Vec3 farNormal = Vec3_Scale(camForward, -1.0f);
    frustum->planes[PLANE_FAR].normal = farNormal;
    frustum->planes[PLANE_FAR].d = -Vec3_Dot(farNormal, farCenter);
    
    // Left plane (usa 3 pontos: camPos, nearLeft, farLeft)
    Vec3 leftNormal = Vec3_Normalize(Vec3_Cross(
        Vec3_Sub(nearLeft, camPos),
        Vec3_Sub(farLeft, camPos)
    ));
    frustum->planes[PLANE_LEFT].normal = leftNormal;
    frustum->planes[PLANE_LEFT].d = -Vec3_Dot(leftNormal, camPos);
    
    // Right plane
    Vec3 rightNormal = Vec3_Normalize(Vec3_Cross(
        Vec3_Sub(farRight, camPos),
        Vec3_Sub(nearRight, camPos)
    ));
    frustum->planes[PLANE_RIGHT].normal = rightNormal;
    frustum->planes[PLANE_RIGHT].d = -Vec3_Dot(rightNormal, camPos);
    
    // Top plane
    Vec3 topNormal = Vec3_Normalize(Vec3_Cross(
        Vec3_Sub(nearTop, camPos),
        Vec3_Sub(farTop, camPos)
    ));
    frustum->planes[PLANE_TOP].normal = topNormal;
    frustum->planes[PLANE_TOP].d = -Vec3_Dot(topNormal, camPos);
    
    // Bottom plane
    Vec3 bottomNormal = Vec3_Normalize(Vec3_Cross(
        Vec3_Sub(farBottom, camPos),
        Vec3_Sub(nearBottom, camPos)
    ));
    frustum->planes[PLANE_BOTTOM].normal = bottomNormal;
    frustum->planes[PLANE_BOTTOM].d = -Vec3_Dot(bottomNormal, camPos);
}

// ============================================================================
// TESTES DE CULLING
// ============================================================================

bool Frustum_IsPointInside(const Frustum* frustum, Vec3 point) {
    if (!frustum) return false;
    
    // Ponto está dentro se está do lado positivo de todos os planos
    for (int i = 0; i < 6; i++) {
        if (Plane_DistanceToPoint(&frustum->planes[i], point) < 0.0f) {
            return false;
        }
    }
    return true;
}

bool Frustum_IsSphereInside(const Frustum* frustum, Vec3 center, float radius) {
    if (!frustum) return false;
    
    // Esfera está dentro se distância ao plano >= -radius para todos os planos
    for (int i = 0; i < 6; i++) {
        float dist = Plane_DistanceToPoint(&frustum->planes[i], center);
        if (dist < -radius) {
            return false;
        }
    }
    return true;
}

bool Frustum_IsAABBInside(const Frustum* frustum, Vec3 min, Vec3 max) {
    if (!frustum) return false;
    
    // Testa AABB contra cada plano
    // Para cada plano, encontra o vértice do AABB mais próximo ao plano
    for (int i = 0; i < 6; i++) {
        const Plane* plane = &frustum->planes[i];
        
        // Vértice mais próximo (ponto negativo da normal)
        Vec3 pVertex = {
            (plane->normal.x < 0.0f) ? min.x : max.x,
            (plane->normal.y < 0.0f) ? min.y : max.y,
            (plane->normal.z < 0.0f) ? min.z : max.z
        };
        
        // Se vértice mais próximo está fora, AABB está fora
        if (Plane_DistanceToPoint(plane, pVertex) < 0.0f) {
            return false;
        }
    }
    return true;
}

float Frustum_DistSqToCamera(const Frustum* frustum, Vec3 point) {
    if (!frustum) return 0.0f;
    return Vec3_DistSq(frustum->camPos, point);
}

// ============================================================================
// CULLING COMBINADO
// ============================================================================

bool Frustum_ShouldRenderChunk(const Frustum* frustum, Vec3 chunkCenter, Vec3 chunkMin, Vec3 chunkMax) {
    if (!frustum) return false;
    
    // 1. Testa distância (evita sqrt usando dist²)
    float distSq = Frustum_DistSqToCamera(frustum, chunkCenter);
    float maxDistSq = frustum->farDist * frustum->farDist;
    if (distSq > maxDistSq) {
        return false; // Muito longe
    }
    
    // 2. Testa frustum
    if (!Frustum_IsAABBInside(frustum, chunkMin, chunkMax)) {
        return false; // Fora do frustum
    }
    
    return true; // Dentro do frustum e dentro da distância
}
