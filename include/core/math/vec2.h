#ifndef VEC2_H
#define VEC2_H

#include <stdint.h>
#include <math.h>

typedef struct {
    float x, y;
} Vec2;

// Operações básicas
static inline Vec2 Vec2_Add(Vec2 a, Vec2 b) {
    return (Vec2){a.x + b.x, a.y + b.y};
}

static inline Vec2 Vec2_Sub(Vec2 a, Vec2 b) {
    return (Vec2){a.x - b.x, a.y - b.y};
}

static inline Vec2 Vec2_Scale(Vec2 a, float s) {
    return (Vec2){a.x * s, a.y * s};
}

static inline float Vec2_Length(Vec2 a) {
    return sqrtf(a.x * a.x + a.y * a.y);
}

static inline Vec2 Vec2_Normalize(Vec2 a) {
    float len = Vec2_Length(a);
    if (len > 0.0001f) {
        return Vec2_Scale(a, 1.0f / len);
    }
    return (Vec2){0, 0};
}

static inline float Vec2_Dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

#endif // VEC2_H
