#include "core/math/mat4.h"
#include <math.h>

Mat4 Mat4_Mul(Mat4 a, Mat4 b) {
    Mat4 result = {0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += a.m[i + k * 4] * b.m[k + j * 4];
            }
            result.m[i + j * 4] = sum;
        }
    }
    return result;
}

Vec3 Mat4_TransformPoint(Mat4 m, Vec3 p) {
    float w = m.m[12] * p.x + m.m[13] * p.y + m.m[14] * p.z + m.m[15];
    if (fabsf(w) < 0.0001f) w = 1.0f;
    return (Vec3){
        (m.m[0] * p.x + m.m[4] * p.y + m.m[8] * p.z + m.m[12]) / w,
        (m.m[1] * p.x + m.m[5] * p.y + m.m[9] * p.z + m.m[13]) / w,
        (m.m[2] * p.x + m.m[6] * p.y + m.m[10] * p.z + m.m[14]) / w
    };
}

Mat4 Mat4_Model(Vec3 pos, Vec3 rot) {
    (void)rot; // TODO: implementar rotação
    // Simplificado: apenas translação por enquanto
    Mat4 m = Mat4_Identity();
    m.m[12] = pos.x;
    m.m[13] = pos.y;
    m.m[14] = pos.z;
    return m;
}

Mat4 Mat4_LookAt(Vec3 eye, Vec3 target, Vec3 up) {
    Vec3 f = Vec3_Normalize(Vec3_Sub(target, eye));
    Vec3 s = Vec3_Normalize(Vec3_Cross(f, up));
    Vec3 u = Vec3_Cross(s, f);
    
    Mat4 m = Mat4_Identity();
    m.m[0] = s.x; m.m[4] = s.y; m.m[8] = s.z;
    m.m[1] = u.x; m.m[5] = u.y; m.m[9] = u.z;
    m.m[2] = -f.x; m.m[6] = -f.y; m.m[10] = -f.z;
    m.m[12] = -Vec3_Dot(s, eye);
    m.m[13] = -Vec3_Dot(u, eye);
    m.m[14] = Vec3_Dot(f, eye);
    return m;
}

Mat4 Mat4_Perspective(float fov, float aspect, float near, float far) {
    float f = 1.0f / tanf(fov * 0.5f);
    Mat4 m = {0};
    m.m[0] = f / aspect;
    m.m[5] = f;
    m.m[10] = (far + near) / (near - far);
    m.m[11] = -1.0f;
    m.m[14] = (2.0f * far * near) / (near - far);
    return m;
}
