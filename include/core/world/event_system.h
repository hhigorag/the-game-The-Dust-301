#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "world_config.h"
#include "segment_manager.h"
#include <stdint.h>

/* ============================================================================
 * EVENT SYSTEM — Evento por segmento (determinístico)
 * eventSeed = Hash(worldSeed, segmentIndex)
 * Eventos: tempestade, falha gravitacional, zona de silêncio, radar glitch.
 * ============================================================================ */

/* Tipos de evento por segmento (ex.: tempestade, glitch, silêncio) */
typedef enum {
    EVENT_NONE = 0,
    EVENT_STORM,
    EVENT_GRAVITY_FAIL,
    EVENT_SILENCE_ZONE,
    EVENT_RADAR_GLITCH,
    EVENT_COUNT
} SegmentEventType;

/* Retorna a seed do evento para o segmento (determinístico). */
uint64_t EventSystem_GetEventSeed(uint64_t worldSeed, int32_t segmentIndex);

/* Retorna o tipo de evento para o segmento (derivado da seed). */
SegmentEventType EventSystem_GetEventForSegment(uint64_t worldSeed, int32_t segmentIndex);

#endif /* EVENT_SYSTEM_H */
