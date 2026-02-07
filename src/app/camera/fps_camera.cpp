#include "app/camera/fps_camera.h"
#include <raylib.h>
#include <cmath>
#include <cstring>

#ifdef __cplusplus
extern "C" {
#endif

void FPSCamera_Init(FPSCamera* cam, float x, float y, float z) {
    if (!cam) return;
    
    cam->position.x = x;
    cam->position.y = y;
    cam->position.z = z;
    cam->yaw = -90.0f; // Inicia olhando para frente (0 graus = olhando para +X, -90 = olhando para +Z)
    cam->pitch = 0.0f;
    cam->fov = 75.0f;
    cam->sensitivity = 0.15f; // Aumentada para melhor responsividade
    cam->locked = false;
    cam->bobOffset.x = cam->bobOffset.y = cam->bobOffset.z = 0.0f;
    cam->roll = 0.0f;
}

void FPSCamera_UpdateMouse(FPSCamera* cam, float mouseDeltaX, float mouseDeltaY) {
    if (!cam || !cam->locked) return;
    
    // Atualiza yaw (rotação horizontal) - mouse para direita = rotaciona para direita
    cam->yaw += mouseDeltaX * cam->sensitivity;
    
    // Normaliza yaw para 0-360
    while (cam->yaw < 0.0f) cam->yaw += 360.0f;
    while (cam->yaw >= 360.0f) cam->yaw -= 360.0f;
    
    // Atualiza pitch (rotação vertical) - mouse para cima = olha para cima
    cam->pitch -= mouseDeltaY * cam->sensitivity;
    
    // Limita pitch entre -89 e 89 graus (estilo Minecraft)
    if (cam->pitch > 89.0f) cam->pitch = 89.0f;
    if (cam->pitch < -89.0f) cam->pitch = -89.0f;
}

void FPSCamera_LockMouse(FPSCamera* cam) {
    if (!cam) return;
    cam->locked = true;
    HideCursor();
    DisableCursor();
}

void FPSCamera_UnlockMouse(FPSCamera* cam) {
    if (!cam) return;
    cam->locked = false;
    EnableCursor();
    ShowCursor();
}

Camera3D FPSCamera_GetRaylibCamera(const FPSCamera* cam) {
    Camera3D result;
    memset(&result, 0, sizeof(Camera3D));
    if (!cam) return result;
    
    // Converte yaw e pitch para radianos
    float yawRad = cam->yaw * 3.14159f / 180.0f;
    float pitchRad = cam->pitch * 3.14159f / 180.0f;
    
    // Calcula direção da câmera (estilo Minecraft)
    float cosPitch = std::cos(pitchRad);
    float forwardX = std::sin(yawRad) * cosPitch;
    float forwardY = std::sin(pitchRad);
    float forwardZ = -std::cos(yawRad) * cosPitch;
    
    // Posição da câmera + head bob
    result.position.x = cam->position.x + cam->bobOffset.x;
    result.position.y = cam->position.y + cam->bobOffset.y;
    result.position.z = cam->position.z + cam->bobOffset.z;
    
    result.target.x = result.position.x + forwardX;
    result.target.y = result.position.y + forwardY;
    result.target.z = result.position.z + forwardZ;
    
    // Up vector com roll (inclinação lateral ao mover o mouse: tilt em torno do eixo forward)
    float rollRad = cam->roll * 3.14159f / 180.0f;
    float cr = std::cos(rollRad), sr = std::sin(rollRad);
    float rightX = -forwardZ, rightZ = forwardX;
    result.up.x = rightX * sr;
    result.up.y = cr;
    result.up.z = rightZ * sr;
    
    result.fovy = cam->fov;
    result.projection = CAMERA_PERSPECTIVE;
    
    return result;
}

void FPSCamera_Move(FPSCamera* cam, float forward, float right, float up, float dt) {
    if (!cam) return;
    
    // Converte yaw para radianos
    float yawRad = cam->yaw * 3.14159f / 180.0f;
    
    // Calcula direções relativas à câmera
    float forwardX = std::sin(yawRad);
    float forwardZ = std::cos(yawRad);
    float rightX = std::cos(yawRad);
    float rightZ = -std::sin(yawRad);
    
    // Aplica movimento
    float speed = 5.0f * dt; // Velocidade base
    
    cam->position.x += (forwardX * forward + rightX * right) * speed;
    cam->position.y += up * speed;
    cam->position.z += (forwardZ * forward + rightZ * right) * speed;
}

void FPSCamera_GetForwardFlat(const FPSCamera* cam, float* outX, float* outZ) {
    if (!cam || !outX || !outZ) return;
    
    // Converte yaw para radianos
    float yawRad = cam->yaw * 3.14159f / 180.0f;
    
    // Forward no plano XZ (sem componente Y)
    // Yaw: -90 = +Z (frente), 0 = +X (direita), 90 = -Z (trás), 180 = -X (esquerda)
    *outX = std::sin(yawRad);
    *outZ = -std::cos(yawRad);
    
    // Normaliza
    float len = std::sqrt(*outX * *outX + *outZ * *outZ);
    if (len > 0.0001f) {
        *outX /= len;
        *outZ /= len;
    }
}

void FPSCamera_GetRightFlat(const FPSCamera* cam, float* outX, float* outZ) {
    if (!cam || !outX || !outZ) return;
    
    // Converte yaw para radianos
    float yawRad = cam->yaw * 3.14159f / 180.0f;
    
    // Right no plano XZ (90 graus à direita do forward)
    *outX = std::cos(yawRad);
    *outZ = std::sin(yawRad);
    
    // Normaliza
    float len = std::sqrt(*outX * *outX + *outZ * *outZ);
    if (len > 0.0001f) {
        *outX /= len;
        *outZ /= len;
    }
}

#ifdef __cplusplus
}
#endif
