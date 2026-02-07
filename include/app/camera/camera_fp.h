#ifndef CAMERA_FP_H
#define CAMERA_FP_H

#include "core/math/core_math.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// CÂMERA FPS ESTILO MINECRAFT
// ============================================================================
// Sistema de câmera em primeira pessoa com yaw/pitch.
// 
// CONVENÇÃO:
// - yaw=0 (graus) = olhando para +Z (frente)
// - yaw=90 = olhando para +X (direita)
// - yaw=-90 = olhando para -X (esquerda)
// - yaw=180 = olhando para -Z (trás)
// - pitch limitado entre -89 e +89 graus
// ============================================================================

typedef struct {
    // Posição da câmera
    Vec3 position;
    
    // Rotação (em graus)
    float yawDeg;    // Rotação horizontal (0 = +Z)
    float pitchDeg;  // Rotação vertical (limitado -89 a +89)
    
    // Configuração
    float fovY;          // Campo de visão vertical (graus)
    float sensitivity;    // Sensibilidade do mouse
    float nearPlane;     // Plano near
    float farPlane;      // Plano far (clampado a 15m)
    
    // Estados
    bool locked;         // Mouse travado?
} CameraFP;

// ============================================================================
// INICIALIZAÇÃO
// ============================================================================

void CameraFP_Init(CameraFP* cam, Vec3 startPos);

// ============================================================================
// ATUALIZAÇÃO COM MOUSE
// ============================================================================

// Atualiza yaw/pitch baseado no delta do mouse
void CameraFP_UpdateMouse(CameraFP* cam, float mouseDeltaX, float mouseDeltaY);

// ============================================================================
// VETORES DE DIREÇÃO
// ============================================================================

// Retorna vetor forward (direção que a câmera está olhando)
Vec3 CameraFP_GetForward(const CameraFP* cam);

// Retorna vetor forward projetado no plano XZ (para movimento)
// IMPORTANTE: Este é usado para movimento W/S (pitch não afeta movimento)
Vec3 CameraFP_GetForwardFlat(const CameraFP* cam);

// Retorna vetor right (direita relativa à câmera)
// IMPORTANTE: Usado para movimento A/D
Vec3 CameraFP_GetRight(const CameraFP* cam);

// Retorna vetor right projetado no plano XZ
Vec3 CameraFP_GetRightFlat(const CameraFP* cam);

// Retorna vetor up (sempre {0, 1, 0})
Vec3 CameraFP_GetUp(const CameraFP* cam);

// ============================================================================
// MOVIMENTO
// ============================================================================

// Calcula direção de movimento desejada baseada em input
// forwardAmount: -1.0 (S) a +1.0 (W)
// rightAmount: -1.0 (A) a +1.0 (D)
// Retorna vetor normalizado no plano XZ
Vec3 CameraFP_CalculateWishDir(const CameraFP* cam, float forwardAmount, float rightAmount);

// ============================================================================
// DEBUG
// ============================================================================

void CameraFP_PrintDebug(const CameraFP* cam);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_FP_H
