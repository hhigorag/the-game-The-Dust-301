#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "core/math/core_math.h"
#include <stdbool.h>

// ============================================================================
// PLAYER CONTROLLER (Física/Movimento Local)
// ============================================================================
// Gerencia posição, velocidade e movimento do player local.
// NOTA: Este é diferente de Player em game_state.h (que é para multiplayer).
// Movimento usa forward_flat (projetado no plano XZ) para que
// pitch não afete movimento horizontal.
// ============================================================================

typedef struct {
    // Posição e velocidade
    Vec3 position;
    Vec3 velocity;
    
    // Propriedades físicas
    float speed;        // Velocidade de movimento (m/s)
    float sprintSpeed; // Velocidade de sprint (m/s)
    float height;       // Altura do player (capsule ou AABB)
    float radius;       // Raio do player (capsule) ou metade da largura (AABB)
    
    // Estados
    bool onGround;      // Está no chão?
    bool isSprinting;    // Está correndo?
    
    // Debug
    bool debugMode;     // Mostra informações de debug?
} PlayerController;

// ============================================================================
// INICIALIZAÇÃO
// ============================================================================

void PlayerController_Init(PlayerController* player, Vec3 startPos);

// ============================================================================
// MOVIMENTO
// ============================================================================
// forward_flat: direção forward projetada no plano XZ (sem componente Y)
// right_flat: direção right projetada no plano XZ
// wishdir: direção desejada de movimento (normalizada)
// ============================================================================

// Atualiza movimento do player baseado em wishdir (direção desejada)
// wishdir deve estar normalizado e no plano XZ
void PlayerController_ApplyMovement(PlayerController* player, Vec3 wishdir, float dt);

// ============================================================================
// FÍSICA
// ============================================================================

// Aplica gravidade
void PlayerController_ApplyGravity(PlayerController* player, float gravity, float dt);

// Atualiza posição baseada na velocidade
void PlayerController_UpdatePosition(PlayerController* player, float dt);

// ============================================================================
// COLISÃO (STUB)
// ============================================================================

// Verifica e resolve colisão com o mundo
// Retorna true se houve colisão
// TODO: Implementar colisão real com chunks/blocos
bool PlayerController_CheckCollision(PlayerController* player, Vec3* newPos);

// ============================================================================
// DEBUG
// ============================================================================

void PlayerController_PrintDebug(const PlayerController* player);

#endif // PLAYER_CONTROLLER_H
