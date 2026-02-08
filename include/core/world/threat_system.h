#ifndef THREAT_SYSTEM_H
#define THREAT_SYSTEM_H

#include "world_config.h"
#include <stdint.h>

/* ============================================================================
 * THREAT SYSTEM — Escalação de perigo
 * threatLevel = base + (distanceFromStart * escalationFactor)
 *               + (engineNoise * noiseFactor)
 *               + (overclockUsage)
 * Quanto mais longe → mais perigo.
 * Quanto mais operador força → mais perigo.
 * ============================================================================ */

typedef struct ThreatSystem ThreatSystem;

struct ThreatSystem {
    float baseLevel;
    float escalationFactor;   /* por metro de distância */
    float noiseFactor;
    float overclockUsage;     /* 0..1, uso de overclock */
    float lastLevel;         /* último nível calculado */
    uint64_t worldSeed;
};

/* Inicializa o sistema com seed e fatores opcionais. */
void ThreatSystem_Init(ThreatSystem* ts, uint64_t worldSeed);

/* Atualiza e retorna o nível de ameaça atual.
 * shipZ: posição Z da nave (metros).
 * overclockUsage: 0..1 (uso de overclock pelo operador). */
float ThreatSystem_Update(ThreatSystem* ts, float shipZ, float overclockUsage);

/* Retorna o nível de ameaça no último Update (sem recalcular). */
float ThreatSystem_GetLevel(const ThreatSystem* ts);

#endif /* THREAT_SYSTEM_H */
