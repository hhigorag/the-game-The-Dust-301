#include "core/gameplay/player.h"
#include <stdio.h>

// Nota: Este arquivo foi renomeado de player.h para player_controller.h
// mas mantém compatibilidade com includes antigos

// ============================================================================
// CONSTANTES
// ============================================================================

#define DEFAULT_SPEED 5.0f
#define DEFAULT_SPRINT_SPEED 8.0f
#define DEFAULT_HEIGHT 1.8f
#define DEFAULT_RADIUS 0.3f

// ============================================================================
// INICIALIZAÇÃO
// ============================================================================

void PlayerController_Init(PlayerController* player, Vec3 startPos) {
    if (!player) return;
    
    player->position = startPos;
    player->velocity = Vec3_Zero();
    player->speed = DEFAULT_SPEED;
    player->sprintSpeed = DEFAULT_SPRINT_SPEED;
    player->height = DEFAULT_HEIGHT;
    player->radius = DEFAULT_RADIUS;
    player->onGround = false;
    player->isSprinting = false;
    player->debugMode = false;
}

// ============================================================================
// MOVIMENTO
// ============================================================================

void PlayerController_ApplyMovement(PlayerController* player, Vec3 wishdir, float dt) {
    if (!player) return;
    
    // Garante que wishdir está normalizado
    Vec3 normalized = Vec3_Normalize(wishdir);
    
    // Calcula velocidade atual (sprint ou normal)
    float currentSpeed = player->isSprinting ? player->sprintSpeed : player->speed;
    
    // Aplica movimento no plano XZ (ignora componente Y de wishdir)
    // Isso garante que pitch não afeta movimento horizontal
    Vec3 move = Vec3_Scale(normalized, currentSpeed * dt);
    
    // Atualiza velocidade horizontal (mantém Y separado para gravidade)
    player->velocity.x = move.x / dt;
    player->velocity.z = move.z / dt;
    
    // Atualiza posição diretamente (colisão será aplicada depois)
    player->position.x += move.x;
    player->position.z += move.z;
}

// ============================================================================
// FÍSICA
// ============================================================================

void PlayerController_ApplyGravity(PlayerController* player, float gravity, float dt) {
    if (!player) return;
    
    // Aplica gravidade apenas se não estiver no chão
    if (!player->onGround) {
        player->velocity.y += gravity * dt;
    } else {
        // Se está no chão, zera velocidade Y (ou aplica atrito)
        player->velocity.y = 0.0f;
    }
}

void PlayerController_UpdatePosition(PlayerController* player, float dt) {
    if (!player) return;
    
    // Atualiza posição baseada na velocidade
    Vec3 delta = Vec3_Scale(player->velocity, dt);
    
    // Verifica colisão antes de mover
    Vec3 newPos = Vec3_Add(player->position, delta);
    
    if (PlayerController_CheckCollision(player, &newPos)) {
        // Colisão detectada, ajusta posição
        // Por enquanto, apenas atualiza (colisão real será implementada depois)
    }
    
    player->position = newPos;
}

// ============================================================================
// COLISÃO (STUB)
// ============================================================================

bool PlayerController_CheckCollision(PlayerController* player, Vec3* newPos) {
    if (!player || !newPos) return false;
    
    // TODO: Implementar colisão real com chunks/blocos
    // Por enquanto, apenas verifica limites básicos do mundo
    // (implementação real deve verificar AABB do player vs blocos sólidos)
    
    // Stub: sempre permite movimento
    return false;
}

// ============================================================================
// DEBUG
// ============================================================================

void PlayerController_PrintDebug(const PlayerController* player) {
    if (!player) return;
    
    printf("[PLAYER] Pos: (%.2f, %.2f, %.2f)\n",
           player->position.x, player->position.y, player->position.z);
    printf("[PLAYER] Vel: (%.2f, %.2f, %.2f)\n",
           player->velocity.x, player->velocity.y, player->velocity.z);
    printf("[PLAYER] Speed: %.2f (sprint: %.2f)\n",
           player->speed, player->sprintSpeed);
    printf("[PLAYER] OnGround: %s\n", player->onGround ? "YES" : "NO");
}
