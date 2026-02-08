#ifndef SHIP_H
#define SHIP_H

#include <raylib.h>
#include <stdbool.h>

/* ============================================================================
 * SHIP — Plataforma móvel; Drop Sequence por F5 (sem cutscene travando o player).
 * Colisão (hull), deck (safe), escada (auto). Player spawna em cima e anda livre.
 * ============================================================================ */

/* Estado da Drop Sequence (parada → F5 inicia descida → flutuando). Não confundir com ShipState em game_state.h (struct). */
typedef enum {
    SHIP_IDLE_HOVER,   /* Parada no ar; F5 inicia pouso. */
    SHIP_DESCENDING,   /* Descendo em curva (ease-out). */
    SHIP_HOVER_READY   /* Já perto do chão, flutuando (plataforma ativa). */
} ShipDropState;

typedef struct Ship {
    Vector3 position;
    Vector3 previousPosition;  /* Posição no início do frame (para delta). */
    float speed;                /* Velocidade (m/s); 0 = parada. */
    float hoverZSpeed;          /* Por enquanto 0; plataforma parada. */
    float moveDirX, moveDirZ;   /* Direção de movimento (normalizada); norte = +Z. */

    /* Delta de movimento neste frame (player herda quando em cima da nave). */
    float deltaX, deltaY, deltaZ;

    /* Drop Sequence: descida em arco (F5). */
    float descendTimer;
    float descendDuration;
    float startHeight;
    float targetHeight;   /* Altura final (ex.: topo do hull ~2.2 m do chão). */
    float descendT;      /* [0,1] progresso da descida (para tilt no draw). */
    Vector3 descendStartPos;  /* Início do arco (alto e atrás). */
    Vector3 descendEndPos;   /* Pouso (perto do chão no corredor). */
    ShipDropState state;

    /* Três zonas distintas: casco (colisão), deck (em pé), escada (subida). */
    BoundingBox hullBox;   /* Casco sólido — colisão. */
    BoundingBox deckBox;   /* Área onde pode ficar em pé; zona segura (oxigênio). */
    BoundingBox ladderBox; /* Área de subida — transição controlada. */
} Ship;

void Ship_Init(Ship* s);
void Ship_Update(Ship* s, float dt);
void Ship_UpdateCollision(Ship* s);
void Ship_Draw(const Ship* s);

/* Retorna true se a nave já pousou (HOVER_READY). */
bool Ship_IsActive(const Ship* s);

#endif /* SHIP_H */
