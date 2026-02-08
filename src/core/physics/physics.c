#include "core/physics/physics.h"
#include <math.h>
#include <string.h>

void PhysicsBody_Init(PhysicsBody* body, float x, float y, float z) {
    if (!body) return;
    memset(body, 0, sizeof(PhysicsBody));
    body->x = x;
    body->y = y;
    body->z = z;
    body->width = 0.6f;
    body->height = PLAYER_HEIGHT;  /* y = pé; maxY = y + height. */
    body->depth = 0.6f;
    body->onGround = false;
    body->isFlying = false;
    body->isSafe = false;
}

void PhysicsBody_ApplyForce(PhysicsBody* body, float fx, float fy, float fz) {
    if (!body) return;
    body->ax += fx;
    body->ay += fy;
    body->az += fz;
}

void PhysicsBody_ApplyMovement(PhysicsBody* body, float moveX, float moveZ, float dt) {
    (void)dt; // Parâmetro reservado para uso futuro
    if (!body) return;
    
    // Movimento direto e responsivo - aplica velocidade diretamente
    float speed = body->onGround ? RUN_SPEED : WALK_SPEED;
    
    // Normaliza direção de movimento
    float moveLen = sqrtf(moveX * moveX + moveZ * moveZ);
    if (moveLen > 0.0001f) {
        moveX /= moveLen;
        moveZ /= moveLen;
    }
    
    // Aplica movimento diretamente à velocidade (não acumula)
    body->vx = moveX * speed;
    body->vz = moveZ * speed;
}

/* Gravidade somente no eixo Y (estilo Minecraft).
 * Não aplica quando onGround: evita “afundar” no chão e ser empurrado de volta.
 * ax e az nunca são alterados aqui – nenhuma força em X/Z. */
void PhysicsBody_ApplyGravity(PhysicsBody* body, float dt) {
    if (!body || body->isFlying) return;
    if (body->onGround) return;
    
    body->ay = GRAVITY;  /* aceleração constante, só Y */
    (void)dt;
}

void PhysicsBody_Update(PhysicsBody* body, float dt) {
    if (!body) return;
    
    /* Integra aceleração → velocidade. Gravidade só em Y (ax, az não usados). */
    body->vx += body->ax * dt;
    body->vy += body->ay * dt;
    body->vz += body->az * dt;
    
    /* Clamp queda: limite estilo Minecraft, só após integrar. */
    if (body->vy < MAX_FALL_SPEED)
        body->vy = MAX_FALL_SPEED;
    
    // Aplica atrito apenas se não houver input (movimento será aplicado depois)
    // O atrito será aplicado no UpdatePhysicsAndCollisions se não houver input
    // Isso permite que o movimento seja mais responsivo
    float friction = body->onGround ? GROUND_FRICTION : AIR_FRICTION;
    // Reduz atrito para movimento mais responsivo
    body->vx *= friction;
    body->vz *= friction;
    
    body->x += body->vx * dt;
    body->y += body->vy * dt;
    body->z += body->vz * dt;
    
    body->ax = 0.0f;
    body->ay = 0.0f;
    body->az = 0.0f;
    
    // onGround será determinado pela detecção de colisão, não resetado aqui
    // Isso evita alternância rápida entre YES/NO
}

bool PhysicsBody_CheckPointCollision(const PhysicsBody* body, float px, float py, float pz) {
    if (!body) return false;
    
    // AABB collision check
    return (px >= body->x - body->width * 0.5f && px <= body->x + body->width * 0.5f &&
            py >= body->y && py <= body->y + body->height &&
            pz >= body->z - body->depth * 0.5f && pz <= body->z + body->depth * 0.5f);
}

/* Resolve colisão em um eixo. Gravidade só em Y: nenhum bloco “puxa” o player.
 * Y: só resolve quando o bloco está realmente abaixo dos pés ou acima da cabeça
 *    (evita paredes empurrarem em Y e causarem sensação de “gravidade” lateral). */
static void resolve_one_axis(PhysicsBody* body, float blockX, float blockY, float blockZ, int axis) {
    float blockMinX = blockX, blockMaxX = blockX + 1.0f;
    float blockMinY = blockY, blockMaxY = blockY + 1.0f;
    float blockMinZ = blockZ, blockMaxZ = blockZ + 1.0f;
    float playerMinX = body->x - body->width * 0.5f, playerMaxX = body->x + body->width * 0.5f;
    float playerMinY = body->y, playerMaxY = body->y + body->height;
    float playerMinZ = body->z - body->depth * 0.5f, playerMaxZ = body->z + body->depth * 0.5f;
    
    if (playerMaxX < blockMinX || playerMinX > blockMaxX ||
        playerMaxY < blockMinY || playerMinY > blockMaxY ||
        playerMaxZ < blockMinZ || playerMinZ > blockMaxZ)
        return;
    
    const float tol = 0.11f;
    
    if (axis == 0) {
        /* Y: só chão (bloco sob os pés). NÃO colide com teto (faces +Y). */
        // Verifica se o bloco está realmente abaixo dos pés do player
        // Usa uma tolerância maior para evitar alternância rápida
        float standing_on = (blockMaxY <= playerMinY + tol);
        if (!standing_on)
            return;
        
        // Só resolve colisão com chão (não com teto)
        body->y = blockMaxY;
        
        // Só marca como onGround se:
        // 1. Está caindo (vy < 0) ou velocidade Y é muito pequena
        // 2. A posição Y está realmente no topo do bloco
        if (body->vy <= 0.1f) { // Permite pequena tolerância para velocidade
            body->vy = 0.0f;
            body->onGround = true;
        }
        return;
    }
    if (axis == 1) {
        if (body->x < blockX + 0.5f) body->x = blockMinX - body->width * 0.5f;
        else body->x = blockMaxX + body->width * 0.5f;
        body->vx = 0.0f;
        return;
    }
    if (axis == 2) {
        if (body->z < blockZ + 0.5f) body->z = blockMinZ - body->depth * 0.5f;
        else body->z = blockMaxZ + body->depth * 0.5f;
        body->vz = 0.0f;
    }
}

void PhysicsBody_ResolveCollision(PhysicsBody* body, float blockX, float blockY, float blockZ) {
    if (!body) return;
    
    float blockMinX = blockX, blockMaxX = blockX + 1.0f;
    float blockMinY = blockY, blockMaxY = blockY + 1.0f;
    float blockMinZ = blockZ, blockMaxZ = blockZ + 1.0f;
    float playerMinX = body->x - body->width * 0.5f, playerMaxX = body->x + body->width * 0.5f;
    float playerMinY = body->y, playerMaxY = body->y + body->height;
    float playerMinZ = body->z - body->depth * 0.5f, playerMaxZ = body->z + body->depth * 0.5f;
    
    if (playerMaxX < blockMinX || playerMinX > blockMaxX ||
        playerMaxY < blockMinY || playerMinY > blockMaxY ||
        playerMaxZ < blockMinZ || playerMinZ > blockMaxZ)
        return;
    
    float overlapX = fminf(playerMaxX - blockMinX, blockMaxX - playerMinX);
    float overlapY = fminf(playerMaxY - blockMinY, blockMaxY - playerMinY);
    float overlapZ = fminf(playerMaxZ - blockMinZ, blockMaxZ - playerMinZ);
    
    if (overlapY <= overlapX && overlapY <= overlapZ)
        resolve_one_axis(body, blockX, blockY, blockZ, 0);
    else if (overlapX < overlapZ)
        resolve_one_axis(body, blockX, blockY, blockZ, 1);
    else
        resolve_one_axis(body, blockX, blockY, blockZ, 2);
}

void PhysicsBody_ResolveCollisionAxis(PhysicsBody* body, float blockX, float blockY, float blockZ, int axis) {
    if (!body || axis < 0 || axis > 2) return;
    resolve_one_axis(body, blockX, blockY, blockZ, axis);
}
