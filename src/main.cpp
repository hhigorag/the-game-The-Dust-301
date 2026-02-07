// ============================================================================
// MAIN.CPP - TESTES E VALIDAÇÃO
// ============================================================================
// Este arquivo contém testes práticos para validar:
// 1. Direção do movimento (W/S/A/D)
// 2. Yaw/Pitch não invertidos
// 3. Pitch não afeta movimento horizontal
// 4. Integração com frustum culling
// ============================================================================

#include "core/math/core_math.h"
#include "core/input/core_input.h"
#include "core/gameplay/player.h"  // PlayerController
#include "app/camera/camera_fp.h"
#include "app/render/frustum.h"
#include "core/world/chunk_system.h"
#include <stdio.h>
#include <math.h>

#ifdef USE_RAYLIB
#include <raylib.h>
#endif

// ============================================================================
// TESTES DE MOVIMENTO
// ============================================================================

void Test_MovementDirection(void) {
    printf("\n=== TESTE 1: DIREÇÃO DE MOVIMENTO ===\n");
    
    Player player;
    CameraFP camera;
    
    // Inicializa player e câmera na origem
    Player_Init(&player, Vec3_Make(0, 0, 0));
    CameraFP_Init(&camera, Vec3_Make(0, 0, 0));
    
    float dt = 1.0f; // 1 segundo
    float speed = 5.0f;
    player.speed = speed;
    
    // Teste 1.1: yaw=0, W deve mover +Z
    printf("\n[Teste 1.1] yaw=0, W por 1s (speed=5):\n");
    camera.yawDeg = 0.0f;
    camera.pitchDeg = 0.0f;
    Vec3 wishdir = CameraFP_CalculateWishDir(&camera, 1.0f, 0.0f); // W
    PlayerController_ApplyMovement(&player, wishdir, dt);
    printf("  Posição esperada: Z ~ +5.0\n");
    printf("  Posição atual: Z = %.2f\n", player.position.z);
    printf("  Wishdir: (%.3f, %.3f, %.3f)\n", wishdir.x, wishdir.y, wishdir.z);
    if (fabsf(player.position.z - 5.0f) < 0.1f) {
        printf("  ✓ PASSOU\n");
    } else {
        printf("  ✗ FALHOU (Z deveria ser ~5.0)\n");
    }
    
    // Reset
    PlayerController_Init(&player, Vec3_Make(0, 0, 0));
    
    // Teste 1.2: yaw=90, W deve mover +X
    printf("\n[Teste 1.2] yaw=90, W por 1s (speed=5):\n");
    camera.yawDeg = 90.0f;
    wishdir = CameraFP_CalculateWishDir(&camera, 1.0f, 0.0f);
    Player_ApplyMovement(&player, wishdir, dt);
    printf("  Posição esperada: X ~ +5.0\n");
    printf("  Posição atual: X = %.2f\n", player.position.x);
    printf("  Wishdir: (%.3f, %.3f, %.3f)\n", wishdir.x, wishdir.y, wishdir.z);
    if (fabsf(player.position.x - 5.0f) < 0.1f) {
        printf("  ✓ PASSOU\n");
    } else {
        printf("  ✗ FALHOU (X deveria ser ~5.0)\n");
    }
    
    // Reset
    PlayerController_Init(&player, Vec3_Make(0, 0, 0));
    
    // Teste 1.3: yaw=-90, W deve mover -X
    printf("\n[Teste 1.3] yaw=-90, W por 1s (speed=5):\n");
    camera.yawDeg = -90.0f;
    wishdir = CameraFP_CalculateWishDir(&camera, 1.0f, 0.0f);
    Player_ApplyMovement(&player, wishdir, dt);
    printf("  Posição esperada: X ~ -5.0\n");
    printf("  Posição atual: X = %.2f\n", player.position.x);
    if (fabsf(player.position.x - (-5.0f)) < 0.1f) {
        printf("  ✓ PASSOU\n");
    } else {
        printf("  ✗ FALHOU (X deveria ser ~-5.0)\n");
    }
    
    // Reset
    PlayerController_Init(&player, Vec3_Make(0, 0, 0));
    
    // Teste 1.4: yaw=180, W deve mover -Z
    printf("\n[Teste 1.4] yaw=180, W por 1s (speed=5):\n");
    camera.yawDeg = 180.0f;
    wishdir = CameraFP_CalculateWishDir(&camera, 1.0f, 0.0f);
    Player_ApplyMovement(&player, wishdir, dt);
    printf("  Posição esperada: Z ~ -5.0\n");
    printf("  Posição atual: Z = %.2f\n", player.position.z);
    if (fabsf(player.position.z - (-5.0f)) < 0.1f) {
        printf("  ✓ PASSOU\n");
    } else {
        printf("  ✗ FALHOU (Z deveria ser ~-5.0)\n");
    }
}

void Test_PitchDoesNotAffectMovement(void) {
    printf("\n=== TESTE 2: PITCH NÃO AFETA MOVIMENTO ===\n");
    
    PlayerController player;
    CameraFP camera;
    
    PlayerController_Init(&player, Vec3_Make(0, 0, 0));
    CameraFP_Init(&camera, Vec3_Make(0, 0, 0));
    
    float dt = 1.0f;
    player.speed = 5.0f;
    
    // Teste: pitch=+60, yaw=0, W deve mover apenas em Z (Y não muda)
    printf("\n[Teste 2.1] pitch=+60, yaw=0, W por 1s:\n");
    camera.yawDeg = 0.0f;
    camera.pitchDeg = 60.0f;
    
    Vec3 forward = CameraFP_GetForward(&camera);
    Vec3 forwardFlat = CameraFP_GetForwardFlat(&camera);
    
    printf("  Forward (com pitch): (%.3f, %.3f, %.3f)\n", forward.x, forward.y, forward.z);
    printf("  ForwardFlat (sem pitch): (%.3f, 0.00, %.3f)\n", forwardFlat.x, forwardFlat.z);
    
    Vec3 wishdir = CameraFP_CalculateWishDir(&camera, 1.0f, 0.0f);
    Player_ApplyMovement(&player, wishdir, dt);
    
    printf("  Posição após movimento: (%.2f, %.2f, %.2f)\n",
           player.position.x, player.position.y, player.position.z);
    
    // Y não deve mudar (movimento é apenas no plano XZ)
    if (fabsf(player.position.y) < 0.01f && fabsf(player.position.z - 5.0f) < 0.1f) {
        printf("  ✓ PASSOU (Y não mudou, Z aumentou)\n");
    } else {
        printf("  ✗ FALHOU (Y deveria ser 0, Z deveria ser ~5.0)\n");
    }
}

void Test_ADNotInverted(void) {
    printf("\n=== TESTE 3: A/D NÃO INVERTIDOS ===\n");
    
    PlayerController player;
    CameraFP camera;
    
    PlayerController_Init(&player, Vec3_Make(0, 0, 0));
    CameraFP_Init(&camera, Vec3_Make(0, 0, 0));
    
    float dt = 1.0f;
    player.speed = 5.0f;
    
    // Teste 3.1: yaw=0, A deve mover X negativo, D deve mover X positivo
    printf("\n[Teste 3.1] yaw=0:\n");
    camera.yawDeg = 0.0f;
    
    Vec3 wishdirA = CameraFP_CalculateWishDir(&camera, 0.0f, -1.0f); // A
    Vec3 wishdirD = CameraFP_CalculateWishDir(&camera, 0.0f, 1.0f);  // D
    
    printf("  Wishdir A: (%.3f, %.3f, %.3f)\n", wishdirA.x, wishdirA.y, wishdirA.z);
    printf("  Wishdir D: (%.3f, %.3f, %.3f)\n", wishdirD.x, wishdirD.y, wishdirD.z);
    
    // A deve ser X negativo
    Player_Init(&player, Vec3_Make(0, 0, 0));
    Player_ApplyMovement(&player, wishdirA, dt);
    printf("  A moveu para: X = %.2f\n", player.position.x);
    if (player.position.x < -4.0f) {
        printf("  ✓ PASSOU (A move para esquerda/X negativo)\n");
    } else {
        printf("  ✗ FALHOU (A deveria mover X negativo)\n");
    }
    
    // D deve ser X positivo
    PlayerController_Init(&player, Vec3_Make(0, 0, 0));
    PlayerController_ApplyMovement(&player, wishdirD, dt);
    printf("  D moveu para: X = %.2f\n", player.position.x);
    if (player.position.x > 4.0f) {
        printf("  ✓ PASSOU (D move para direita/X positivo)\n");
    } else {
        printf("  ✗ FALHOU (D deveria mover X positivo)\n");
    }
    
    // Teste 3.2: yaw=90, A/D devem ser relativos ao olhar
    printf("\n[Teste 3.2] yaw=90:\n");
    camera.yawDeg = 90.0f;
    
    wishdirA = CameraFP_CalculateWishDir(&camera, 0.0f, -1.0f);
    wishdirD = CameraFP_CalculateWishDir(&camera, 0.0f, 1.0f);
    
    printf("  Wishdir A: (%.3f, %.3f, %.3f)\n", wishdirA.x, wishdirA.y, wishdirA.z);
    printf("  Wishdir D: (%.3f, %.3f, %.3f)\n", wishdirD.x, wishdirD.y, wishdirD.z);
    printf("  (A/D devem ser relativos ao olhar, não absolutos)\n");
}

void Test_FrustumCulling(void) {
    printf("\n=== TESTE 4: FRUSTUM CULLING ===\n");
    
    Frustum frustum;
    CameraFP camera;
    
    CameraFP_Init(&camera, Vec3_Make(0, 0, 0));
    camera.yawDeg = 0.0f;
    camera.pitchDeg = 0.0f;
    
    Vec3 camForward = CameraFP_GetForward(&camera);
    Vec3 camUp = CameraFP_GetUp(&camera);
    
    float fovY = DegToRad(75.0f);
    float aspect = 16.0f / 9.0f;
    float near = 0.1f;
    float far = 20.0f;
    
    Frustum_Calculate(&frustum, camera.position, camForward, camUp, fovY, aspect, near, far);
    
    printf("  Far efetivo (15m + margem): %.2f\n", frustum.farDist);
    
    // Testa ponto dentro do frustum
    Vec3 pointInside = Vec3_Make(0, 0, 5.0f);
    bool inside = Frustum_IsPointInside(&frustum, pointInside);
    printf("  Ponto (0, 0, 5) dentro: %s\n", inside ? "SIM" : "NÃO");
    
    // Testa ponto fora do frustum (muito longe)
    Vec3 pointFar = Vec3_Make(0, 0, 20.0f);
    bool farInside = Frustum_IsPointInside(&frustum, pointFar);
    printf("  Ponto (0, 0, 20) dentro: %s (deve ser NÃO)\n", farInside ? "SIM" : "NÃO");
}

void Test_ChunkCulling(void) {
    printf("\n=== TESTE 5: CHUNK CULLING ===\n");
    
    Frustum frustum;
    CameraFP camera;
    
    CameraFP_Init(&camera, Vec3_Make(0, 64, 0));
    Vec3 camForward = CameraFP_GetForward(&camera);
    Vec3 camUp = CameraFP_GetUp(&camera);
    
    Frustum_Calculate(&frustum, camera.position, camForward, camUp,
                     DegToRad(75.0f), 16.0f/9.0f, 0.1f, 20.0f);
    
    ChunkInfo chunks[100];
    int chunkCount = ChunkSystem_GetChunksInRadius(chunks, 100, camera.position, 2);
    
    printf("  Chunks encontrados: %d\n", chunkCount);
    
    int rendered = 0;
    for (int i = 0; i < chunkCount; i++) {
        if (Frustum_ShouldRenderChunk(&frustum, chunks[i].center, chunks[i].min, chunks[i].max)) {
            rendered++;
        }
    }
    
    printf("  Chunks a renderizar: %d\n", rendered);
}

// ============================================================================
// MAIN
// ============================================================================

int main(void) {
    printf("========================================\n");
    printf("TESTES DE VALIDAÇÃO - GAMEPLAY ENGINE\n");
    printf("========================================\n");
    
    // Executa todos os testes
    Test_MovementDirection();
    Test_PitchDoesNotAffectMovement();
    Test_ADNotInverted();
    Test_FrustumCulling();
    Test_ChunkCulling();
    
    printf("\n========================================\n");
    printf("TESTES CONCLUÍDOS\n");
    printf("========================================\n");
    
    return 0;
}
