#ifndef WORLD_CONFIG_H
#define WORLD_CONFIG_H

#include <stdint.h>

/* ============================================================================
 * BEWARE THE DUST — Estrutura do mundo (linear, guiado, streaming)
 * ============================================================================ */

/* Mapa */
#define WORLD_WIDTH_M        1000   /* Largura (X): -500 a +500 m */
#define WORLD_LENGTH_M       4000   /* Comprimento total (Z): 0 a 4000 m */
#define WORLD_GOAL_Z_M       3600   /* Objetivo final (m) */
#define WORLD_DEAD_ZONE_Z_M  3600   /* 3600–4000: área morta pós-extração */

/* Chunks (Minecraft-style, linear). 1 chunk = 32 m. */
#define CHUNK_SIZE_M         32
#define CHUNKS_WIDE           31    /* 1000/32 ≈ 31 */
#define CHUNKS_LONG           125   /* 4000/32 = 125 */
#define CHUNK_Z_GOAL          112   /* 3600/32 = 112.5 → chunk 112 */

/* Nave: pivô do streaming */
#define SHIP_SPEED_MPS        1.5f
#define ROUND_DURATION_S      480
#define ROUND_DISTANCE_M      (SHIP_SPEED_MPS * ROUND_DURATION_S)  /* 720 m */

/* Streaming: 6 atrás, 18 à frente ≈ 24 chunks ≈ 768 m (cobre ~700 m visível) */
#define STREAM_CHUNKS_BEHIND  6
#define STREAM_CHUNKS_AHEAD   18
#define STREAM_CHUNKS_TOTAL   (STREAM_CHUNKS_BEHIND + 1 + STREAM_CHUNKS_AHEAD)

/* Zonas macro (em metros). O ritmo é autoral; o mundo é procedural. */
#define ZONE_STABLE_END       600   /* 0–600: Zona Estável (introdução) */
#define ZONE_RUINS_END        1400  /* 600–1400: Ruínas densas */
#define ZONE_OPEN_END         2200  /* 1400–2200: Área aberta / tempestades */
#define ZONE_UNDERGROUND_END  3000  /* 2200–3000: Complexo subterrâneo */
#define ZONE_FINAL_END        3600  /* 3000–3600: Zona final (caos) */
#define ZONE_DEAD_END         4000  /* 3600–4000: Área morta */

/* Segmento dramático: ~160 m = 5 chunks por segmento */
#define CHUNKS_PER_SEGMENT    5
#define SEGMENT_COUNT         ((CHUNKS_LONG + CHUNKS_PER_SEGMENT - 1) / CHUNKS_PER_SEGMENT)

/* Corridor bias: deriva do eixo central (metros). Noise * 150. */
#define CORRIDOR_DRIFT_MAX_M  150

/* Estruturas grandes: a cada 300–500 m ≈ 10–16 chunks */
#define LARGE_STRUCTURE_MIN_CHUNKS  10
#define LARGE_STRUCTURE_MAX_CHUNKS  16

/* Final 400 m: clímax (O₂, eventos, engine, stealth ignorado) */
#define FINAL_CLIMAX_START_Z_M 3200

/* Tipos de segmento (macro layer) */
typedef enum {
    SEGMENT_STABLE = 0,           /* Zona estável (introdução) */
    SEGMENT_URBAN_DENSE,          /* Ruínas densas */
    SEGMENT_INDUSTRIAL,           /* Área aberta / tempestades */
    SEGMENT_OPEN_DESERT,          /* Complexo subterrâneo */
    SEGMENT_COLLAPSED_STRUCTURE,  /* Zona final (caos) */
    SEGMENT_FINAL_CORRIDOR,       /* Corredor final */
    SEGMENT_DEAD_ZONE,            /* Área morta pós-extração */
    SEGMENT_TYPE_COUNT
} SegmentType;

/* Converte coordenada mundo Z (metros) em índice de chunk Z (0..CHUNKS_LONG-1) */
static inline int32_t WorldZToChunkZ(float worldZ) {
    int32_t cz = (int32_t)(worldZ / (float)CHUNK_SIZE_M);
    if (cz < 0) return 0;
    if (cz >= CHUNKS_LONG) return CHUNKS_LONG - 1;
    return cz;
}

/* Converte coordenada mundo X (metros) em índice de chunk X (-CHUNKS_WIDE/2 .. +CHUNKS_WIDE/2) */
static inline int32_t WorldXToChunkX(float worldX) {
    int32_t cx = (int32_t)(worldX / (float)CHUNK_SIZE_M);
    if (cx < -CHUNKS_WIDE/2) return -CHUNKS_WIDE/2;
    if (cx > CHUNKS_WIDE/2) return CHUNKS_WIDE/2;
    return cx;
}

/* Converte chunk Z em distância aproximada em metros (centro do chunk) */
static inline float ChunkZToWorldZ(int32_t chunkZ) {
    return (float)(chunkZ * CHUNK_SIZE_M) + (float)CHUNK_SIZE_M * 0.5f;
}

#endif /* WORLD_CONFIG_H */
