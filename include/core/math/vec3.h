#ifndef VEC3_H
#define VEC3_H

#include <stdint.h>
#include <math.h>

typedef struct {
    float x, y, z;
} Vec3;

// Operações básicas
static inline Vec3 Vec3_Add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 Vec3_Sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vec3 Vec3_Scale(Vec3 a, float s) {
    return (Vec3){a.x * s, a.y * s, a.z * s};
}

static inline float Vec3_Length(Vec3 a) {
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

static inline Vec3 Vec3_Normalize(Vec3 a) {
    float len = Vec3_Length(a);
    if (len > 0.0001f) {
        return Vec3_Scale(a, 1.0f / len);
    }
    return (Vec3){0, 0, 0};
}

static inline float Vec3_Dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 Vec3_Cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

// Conversão graus ↔ radianos
static inline float DegToRad(float deg) {
    return deg * 3.14159265359f / 180.0f;
}

static inline float RadToDeg(float rad) {
    return rad * 180.0f / 3.14159265359f;
}

// Yaw/Pitch → forward vector
static inline Vec3 Vec3_FromYawPitch(float yaw, float pitch) {
    float cy = cosf(yaw);
    float sy = sinf(yaw);
    float cp = cosf(pitch);
    float sp = sinf(pitch);
    return (Vec3){
        cy * cp,
        sp,
        sy * cp
    };
}

#endif // VEC3_H
