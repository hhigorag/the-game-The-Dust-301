#include "app/camera/camera_fp.h"
#include <stdio.h>
#include <math.h>

// ============================================================================
// CONSTANTES
// ============================================================================

#define DEFAULT_FOV 75.0f
#define DEFAULT_SENSITIVITY 0.15f
#define DEFAULT_NEAR 0.1f
#define DEFAULT_FAR 15.0f  // Clampado a 15m
#define PITCH_MIN -89.0f
#define PITCH_MAX 89.0f

// ============================================================================
// INICIALIZAÇÃO
// ============================================================================

void CameraFP_Init(CameraFP* cam, Vec3 startPos) {
    if (!cam) return;
    
    cam->position = startPos;
    cam->yawDeg = 0.0f;      // Olhando para +Z
    cam->pitchDeg = 0.0f;    // Olhando horizontalmente
    cam->fovY = DEFAULT_FOV;
    cam->sensitivity = DEFAULT_SENSITIVITY;
    cam->nearPlane = DEFAULT_NEAR;
    cam->farPlane = DEFAULT_FAR;
    cam->locked = false;
}

// ============================================================================
// ATUALIZAÇÃO COM MOUSE
// ============================================================================

void CameraFP_UpdateMouse(CameraFP* cam, float mouseDeltaX, float mouseDeltaY) {
    if (!cam || !cam->locked) return;
    
    // Atualiza yaw (rotação horizontal)
    // Mouse para direita = rotaciona para direita (yaw aumenta)
    cam->yawDeg += mouseDeltaX * cam->sensitivity;
    
    // Normaliza yaw para 0-360
    while (cam->yawDeg < 0.0f) cam->yawDeg += 360.0f;
    while (cam->yawDeg >= 360.0f) cam->yawDeg -= 360.0f;
    
    // Atualiza pitch (rotação vertical)
    // Mouse para cima = olha para cima (pitch diminui, por isso inverte Y)
    cam->pitchDeg -= mouseDeltaY * cam->sensitivity;
    
    // Limita pitch entre -89 e +89 graus (estilo Minecraft)
    if (cam->pitchDeg > PITCH_MAX) cam->pitchDeg = PITCH_MAX;
    if (cam->pitchDeg < PITCH_MIN) cam->pitchDeg = PITCH_MIN;
}

// ============================================================================
// VETORES DE DIREÇÃO
// ============================================================================

Vec3 CameraFP_GetForward(const CameraFP* cam) {
    if (!cam) return Vec3_Make(0, 0, 1); // Default: +Z
    
    float yawRad = DegToRad(cam->yawDeg);
    float pitchRad = DegToRad(cam->pitchDeg);
    
    // Fórmula: forward = { sin(yaw)*cos(pitch), sin(pitch), cos(yaw)*cos(pitch) }
    // Com yaw=0, pitch=0: forward = {0, 0, 1} = +Z ✓
    float cosPitch = cosf(pitchRad);
    float sinPitch = sinf(pitchRad);
    float cosYaw = cosf(yawRad);
    float sinYaw = sinf(yawRad);
    
    return Vec3_Make(
        sinYaw * cosPitch,  // X
        sinPitch,           // Y
        cosYaw * cosPitch   // Z (frente quando yaw=0)
    );
}

Vec3 CameraFP_GetForwardFlat(const CameraFP* cam) {
    if (!cam) return Vec3_Make(0, 0, 1); // Default: +Z
    
    // Projeta forward no plano XZ (remove componente Y)
    Vec3 forward = CameraFP_GetForward(cam);
    return Vec3_Normalize(Vec3_ProjectXZ(forward));
}

Vec3 CameraFP_GetRight(const CameraFP* cam) {
    if (!cam) return Vec3_Make(1, 0, 0); // Default: +X
    
    Vec3 forward = CameraFP_GetForward(cam);
    Vec3 up = CameraFP_GetUp(cam);
    
    // Cross product: right = forward × up (right-handed)
    // IMPORTANTE: Ordem do cross product define se A/D estão corretos
    Vec3 right = Vec3_Cross(forward, up);
    return Vec3_Normalize(right);
}

Vec3 CameraFP_GetRightFlat(const CameraFP* cam) {
    if (!cam) return Vec3_Make(1, 0, 0); // Default: +X
    
    // Projeta right no plano XZ
    Vec3 right = CameraFP_GetRight(cam);
    return Vec3_Normalize(Vec3_ProjectXZ(right));
}

Vec3 CameraFP_GetUp(const CameraFP* cam) {
    (void)cam; // Não usado, mas mantém consistência
    return Vec3_Make(0, 1, 0); // Sempre aponta para cima
}

// ============================================================================
// MOVIMENTO
// ============================================================================

Vec3 CameraFP_CalculateWishDir(const CameraFP* cam, float forwardAmount, float rightAmount) {
    if (!cam) return Vec3_Zero();
    
    // Obtém direções no plano XZ (pitch não afeta movimento)
    Vec3 forwardFlat = CameraFP_GetForwardFlat(cam);
    Vec3 rightFlat = CameraFP_GetRightFlat(cam);
    
    // Calcula direção desejada
    Vec3 wishdir = Vec3_Add(
        Vec3_Scale(forwardFlat, forwardAmount),
        Vec3_Scale(rightFlat, rightAmount)
    );
    
    // Normaliza (importante para movimento diagonal)
    return Vec3_Normalize(wishdir);
}

// ============================================================================
// DEBUG
// ============================================================================

void CameraFP_PrintDebug(const CameraFP* cam) {
    if (!cam) return;
    
    Vec3 forward = CameraFP_GetForward(cam);
    Vec3 forwardFlat = CameraFP_GetForwardFlat(cam);
    Vec3 rightFlat = CameraFP_GetRightFlat(cam);
    
    printf("[CAMERA] Pos: (%.2f, %.2f, %.2f)\n",
           cam->position.x, cam->position.y, cam->position.z);
    printf("[CAMERA] Yaw: %.2f deg, Pitch: %.2f deg\n",
           cam->yawDeg, cam->pitchDeg);
    printf("[CAMERA] Forward: (%.3f, %.3f, %.3f)\n",
           forward.x, forward.y, forward.z);
    printf("[CAMERA] ForwardFlat: (%.3f, 0.00, %.3f)\n",
           forwardFlat.x, forwardFlat.z);
    printf("[CAMERA] RightFlat: (%.3f, 0.00, %.3f)\n",
           rightFlat.x, rightFlat.z);
}
