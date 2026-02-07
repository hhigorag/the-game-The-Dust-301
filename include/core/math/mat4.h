#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"
#include <string.h>

// Matriz 4x4 (column-major, OpenGL style)
typedef struct {
    float m[16]; // [0-3] col0, [4-7] col1, [8-11] col2, [12-15] col3
} Mat4;

// Matriz identidade
static inline Mat4 Mat4_Identity(void) {
    Mat4 m = {0};
    m.m[0] = 1.0f; m.m[5] = 1.0f; m.m[10] = 1.0f; m.m[15] = 1.0f;
    return m;
}

// Multiplicação de matrizes
Mat4 Mat4_Mul(Mat4 a, Mat4 b);

// Transformar ponto Vec3
Vec3 Mat4_TransformPoint(Mat4 m, Vec3 p);

// Matriz Model (posição + rotação)
Mat4 Mat4_Model(Vec3 pos, Vec3 rot);

// Matriz View (LookAt)
Mat4 Mat4_LookAt(Vec3 eye, Vec3 target, Vec3 up);

// Matriz Projection (perspectiva)
Mat4 Mat4_Perspective(float fov, float aspect, float near, float far);

#endif // MAT4_H
