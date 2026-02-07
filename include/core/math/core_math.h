#ifndef CORE_MATH_H
#define CORE_MATH_H

#include <stdint.h>
#include <math.h>

// ============================================================================
// CONVENÇÃO DE COORDENADAS (OBRIGATÓRIA)
// ============================================================================
// Mundo: X = direita, Y = cima, Z = frente
// "Frente" do mundo é +Z
// Sistema right-handed (mão direita)
// ============================================================================

// Vec3: vetor 3D básico
typedef struct {
    float x, y, z;
} Vec3;

// Mat4: matriz 4x4 (para transformações futuras)
typedef struct {
    float m[16]; // Column-major order
} Mat4;

// ============================================================================
// OPERAÇÕES VEC3
// ============================================================================

static inline Vec3 Vec3_Make(float x, float y, float z) {
    return (Vec3){x, y, z};
}

static inline Vec3 Vec3_Zero(void) {
    return (Vec3){0.0f, 0.0f, 0.0f};
}

static inline Vec3 Vec3_Add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 Vec3_Sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vec3 Vec3_Scale(Vec3 v, float s) {
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

static inline float Vec3_Dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 Vec3_Cross(Vec3 a, Vec3 b) {
    // Cross product: a × b (right-handed)
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static inline float Vec3_LengthSq(Vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

static inline float Vec3_Length(Vec3 v) {
    return sqrtf(Vec3_LengthSq(v));
}

static inline Vec3 Vec3_Normalize(Vec3 v) {
    float len = Vec3_Length(v);
    if (len > 0.0001f) {
        return Vec3_Scale(v, 1.0f / len);
    }
    return Vec3_Zero();
}

// Projeta vetor no plano XZ (remove componente Y)
static inline Vec3 Vec3_ProjectXZ(Vec3 v) {
    return (Vec3){v.x, 0.0f, v.z};
}

// ============================================================================
// CONVERSÃO GRAUS ↔ RADIANOS
// ============================================================================

static inline float DegToRad(float deg) {
    return deg * 3.14159265359f / 180.0f;
}

static inline float RadToDeg(float rad) {
    return rad * 180.0f / 3.14159265359f;
}

// ============================================================================
// YAW/PITCH → FORWARD VECTOR
// ============================================================================
// Fórmula: forward = { sin(yaw)*cos(pitch), sin(pitch), cos(yaw)*cos(pitch) }
// Com yaw=0 (radianos), pitch=0: forward = {0, 0, 1} = +Z (frente)
// ============================================================================

static inline Vec3 Vec3_FromYawPitch(float yawRad, float pitchRad) {
    float cosPitch = cosf(pitchRad);
    float sinPitch = sinf(pitchRad);
    float cosYaw = cosf(yawRad);
    float sinYaw = sinf(yawRad);
    
    return (Vec3){
        sinYaw * cosPitch,  // X
        sinPitch,           // Y
        cosYaw * cosPitch   // Z (frente quando yaw=0)
    };
}

// ============================================================================
// DISTÂNCIA AO QUADRADO (evita sqrt)
// ============================================================================

static inline float Vec3_DistSq(Vec3 a, Vec3 b) {
    Vec3 diff = Vec3_Sub(a, b);
    return Vec3_LengthSq(diff);
}

#endif // CORE_MATH_H
