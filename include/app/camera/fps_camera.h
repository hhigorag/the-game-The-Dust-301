#ifndef FPS_CAMERA_H
#define FPS_CAMERA_H

#include <raylib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Câmera FPS estilo Minecraft
typedef struct {
    Vector3 position;      // Posição da câmera
    float yaw;            // Rotação horizontal (graus)
    float pitch;          // Rotação vertical (graus)
    float fov;            // Campo de visão
    float sensitivity;     // Sensibilidade do mouse
    bool locked;          // Mouse travado?
    Vector3 bobOffset;    // Offset de head bob (andar/correr) aplicado à view
    float roll;           // Inclinação lateral (graus) por movimento do mouse, decay
} FPSCamera;

// Inicializa câmera FPS
void FPSCamera_Init(FPSCamera* cam, float x, float y, float z);

// Atualiza câmera com movimento do mouse
void FPSCamera_UpdateMouse(FPSCamera* cam, float mouseDeltaX, float mouseDeltaY);

// Trava/destrava mouse
void FPSCamera_LockMouse(FPSCamera* cam);
void FPSCamera_UnlockMouse(FPSCamera* cam);

// Retorna Camera3D do Raylib
Camera3D FPSCamera_GetRaylibCamera(const FPSCamera* cam);

// Move câmera relativo à sua direção
void FPSCamera_Move(FPSCamera* cam, float forward, float right, float up, float dt);

// Obtém vetor forward no plano XZ (sem componente Y) - para movimento horizontal
void FPSCamera_GetForwardFlat(const FPSCamera* cam, float* outX, float* outZ);

// Obtém vetor right no plano XZ (sem componente Y) - para movimento horizontal
void FPSCamera_GetRightFlat(const FPSCamera* cam, float* outX, float* outZ);

#ifdef __cplusplus
}
#endif

#endif // FPS_CAMERA_H
