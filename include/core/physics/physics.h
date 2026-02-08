#ifndef PHYSICS_H
#define PHYSICS_H

#include <stdint.h>
#include <stdbool.h>

// Estrutura de física do player
typedef struct {
    float x, y, z;          // Posição
    float vx, vy, vz;       // Velocidade
    float ax, ay, az;       // Aceleração
    float width, height, depth; // Dimensões do AABB
    bool onGround;          // Está no chão?
    bool isFlying;          // Modo voo (debug)
    bool isSafe;            // Dentro da zona segura da nave (deck; oxigênio)
} PhysicsBody;

/* Player: y = pé (base do AABB); minY = y, maxY = y + PLAYER_HEIGHT. */
#define PLAYER_HEIGHT      1.80f   /* Altura total do corpo. */
#define PLAYER_EYE_HEIGHT  1.62f   /* Altura dos olhos (câmera) ~90% da altura. */

/* Física estilo Minecraft: gravidade só no eixo Y */
#define GRAVITY -32.0f        /* ~Minecraft: blocks/s² para baixo */
#define MAX_FALL_SPEED -78.0f /* ~Minecraft: velocidade máxima de queda */
#define GROUND_FRICTION 0.6f  /* Atrito no chão */
#define AIR_FRICTION 0.91f    /* Atrito no ar */
#define JUMP_FORCE 9.0f       /* Força do pulo (~Minecraft) */
#define WALK_SPEED 4.3f       /* Caminhada */
#define RUN_SPEED 5.6f        /* Corrida */

// Inicializa um corpo físico
void PhysicsBody_Init(PhysicsBody* body, float x, float y, float z);

// Aplica força ao corpo
void PhysicsBody_ApplyForce(PhysicsBody* body, float fx, float fy, float fz);

// Aplica movimento horizontal (WASD)
void PhysicsBody_ApplyMovement(PhysicsBody* body, float moveX, float moveZ, float dt);

// Aplica gravidade
void PhysicsBody_ApplyGravity(PhysicsBody* body, float dt);

// Atualiza física (integração)
void PhysicsBody_Update(PhysicsBody* body, float dt);

// Verifica colisão AABB vs ponto
bool PhysicsBody_CheckPointCollision(const PhysicsBody* body, float px, float py, float pz);

/* Resolve colisão e ajusta posição */
void PhysicsBody_ResolveCollision(PhysicsBody* body, float blockX, float blockY, float blockZ);

/* Resolve apenas um eixo (Y=0, X=1, Z=2). Evita “puxar” entre eixos. */
void PhysicsBody_ResolveCollisionAxis(PhysicsBody* body, float blockX, float blockY, float blockZ, int axis);

#endif // PHYSICS_H
