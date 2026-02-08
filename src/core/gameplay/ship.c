#include "core/gameplay/ship.h"
#if defined(USE_RLGL)
#include <rlgl.h>
#endif
#include <math.h>

/* EPS de superfície: evita z-fighting em face-against-face (deck/hull, teto/paredes, etc.). */
#define SURFACE_EPS       0.01f

/* Plataforma única: 5×5×1 m (largura × comprimento × espessura). */
#define PLATFORM_HALF_X   2.5f
#define PLATFORM_HALF_Y   0.5f
#define PLATFORM_HALF_Z   2.5f
/* Vibração no final da descida: últimos 15% (descendT > 0.85). */
#define SHIP_SHAKE_THRESHOLD   0.85f
#define SHIP_SHAKE_AMPLITUDE   0.2f
/* Desaceleração horizontal: início 60%, acelera até 100% no fim. */
#define SHIP_SPEED_FACTOR_MIN  0.6f
#define SHIP_SPEED_FACTOR_MAX  1.0f
/* Delay após pouso antes de liberar controle (micro “respiro”). */
#define SHIP_ACTIVATION_DELAY  0.5f
/* Áudio (quando houver sistema de som): volume ∝ (1 - descendT), pitch reduz perto do solo. */

/* Altura do terreno em (x, z). Chão plano y=0; futuro: raycast no VoxelWorld. */
static float GetGroundHeightAt(float x, float z) {
    (void)x;
    (void)z;
    return 0.0f;
}

/* Ease-out exponencial: começa descendo mais rápido, suaviza perto do chão. */
static float EaseOutExpo(float t) {
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return 1.0f - powf(2.0f, -10.0f * t);
}

/* Z do início do arco de pouso (atrás = -Z); landZ fixo = 0 (centro do corredor). */
#define SHIP_DESCEND_START_Z  (-30.0f)

void Ship_Init(Ship* s) {
    if (!s) return;
    /* Nave e player começam onde o pouso começa (início do arco: alto e atrás). */
    s->position = (Vector3){ 0.0f, 20.0f, SHIP_DESCEND_START_Z };
    s->previousPosition = s->position;
    s->deltaX = 0.0f;
    s->deltaY = 0.0f;
    s->deltaZ = 0.0f;
    s->speed = 0.0f;          /* parada no começo; F5 inicia pouso; F6 norte. */
    s->hoverZSpeed = 0.0f;    /* por enquanto plataforma parada */
    s->moveDirX = 0.0f;
    s->moveDirZ = 0.0f;

    s->descendTimer = 0.0f;
    s->descendDuration = 6.0f;   /* 6 segundos de descida */
    s->startHeight = 20.0f;
    s->targetHeight = 2.2f;     /* flutuando baixo o suficiente pra subir (topo hull ~2.2 m do chão) */
    s->descendT = 0.0f;
    s->state = SHIP_IDLE_HOVER;

    Ship_UpdateCollision(s);
}

void Ship_Update(Ship* s, float dt) {
    if (!s) return;

    s->previousPosition = s->position;
    s->deltaX = 0.0f;
    s->deltaY = 0.0f;
    s->deltaZ = 0.0f;

    /* Movimento horizontal só quando HOVER_READY e speed > 0 (ex.: F6 norte). */
    if (s->state == SHIP_HOVER_READY && s->speed != 0.0f) {
        s->position.x += s->moveDirX * s->speed * dt;
        s->position.z += s->moveDirZ * s->speed * dt;
    }

    if (s->state == SHIP_DESCENDING) {
        s->descendTimer += dt;
        float t = s->descendTimer / s->descendDuration;
        if (t > 1.0f) t = 1.0f;
        s->descendT = t;

        float p = EaseOutExpo(t);
        float py = EaseOutExpo(t);

        s->position.x = s->descendStartPos.x + (s->descendEndPos.x - s->descendStartPos.x) * p;
        s->position.z = s->descendStartPos.z + (s->descendEndPos.z - s->descendStartPos.z) * p;
        s->position.y = s->descendStartPos.y + (s->descendEndPos.y - s->descendStartPos.y) * py;

        if (t >= 1.0f) {
            s->position = s->descendEndPos;
            s->state = SHIP_HOVER_READY;
        }
    }

    /* Nave nunca atravessa o chão. */
    {
        float groundY = GetGroundHeightAt(s->position.x, s->position.z);
        float minAllowedY = groundY + s->targetHeight;
        if (s->position.y < minAllowedY) s->position.y = minAllowedY;
    }

    s->deltaX = s->position.x - s->previousPosition.x;
    s->deltaY = s->position.y - s->previousPosition.y;
    s->deltaZ = s->position.z - s->previousPosition.z;
}

void Ship_UpdateCollision(Ship* s) {
    if (!s) return;

    float x = s->position.x;
    float y = s->position.y;
    float z = s->position.z;

    /* Plataforma única (5×5×1). */
    s->hullBox = (BoundingBox){
        { x - PLATFORM_HALF_X, y - PLATFORM_HALF_Y, z - PLATFORM_HALF_Z },
        { x + PLATFORM_HALF_X, y + PLATFORM_HALF_Y, z + PLATFORM_HALF_Z }
    };
    /* Deck = hull (standing em cima da plataforma). */
    s->deckBox = s->hullBox;
    /* Escada desativada (degenerada). */
    s->ladderBox.min = s->position;
    s->ladderBox.max = s->position;
}

void Ship_Draw(const Ship* s) {
    if (!s) return;

    /* Plataforma única 5×5×1, sem rotação. */
#if defined(USE_RLGL)
    rlDrawRenderBatchActive();
    rlPushMatrix();
    rlTranslatef(s->position.x, s->position.y, s->position.z);
    DrawCube((Vector3){ 0.0f, 0.0f, 0.0f }, PLATFORM_HALF_X * 2.0f, PLATFORM_HALF_Y * 2.0f, PLATFORM_HALF_Z * 2.0f, DARKGRAY);
    rlPopMatrix();
    rlDrawRenderBatchActive();
#else
    DrawCube(s->position, PLATFORM_HALF_X * 2.0f, PLATFORM_HALF_Y * 2.0f, PLATFORM_HALF_Z * 2.0f, DARKGRAY);
#endif
}

bool Ship_IsActive(const Ship* s) {
    return s && s->state == SHIP_HOVER_READY;
}
