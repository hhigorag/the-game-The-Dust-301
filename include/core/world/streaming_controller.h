#ifndef STREAMING_CONTROLLER_H
#define STREAMING_CONTROLLER_H

#include "world_config.h"
#include <stdint.h>

/* ============================================================================
 * STREAMING CONTROLLER — A nave é o pivô
 * Carrega 6 chunks atrás, 18 à frente. Descarrega o resto.
 * ============================================================================ */

typedef struct StreamingController StreamingController;

struct StreamingController {
    int32_t currentChunkZ;   /* chunk Z atual da nave */
    int32_t minChunkZ;       /* mínimo a manter carregado */
    int32_t maxChunkZ;       /* máximo a manter carregado */
};

/* Inicializa o controller. */
void StreamingController_Init(StreamingController* sc);

/* Atualiza o range de chunks a manter carregados com base na posição Z da nave (metros). */
void StreamingController_Update(StreamingController* sc, float shipWorldZ);

/* Retorna o chunk Z mínimo que deve estar carregado (inclusive). */
int32_t StreamingController_GetMinChunkZ(const StreamingController* sc);

/* Retorna o chunk Z máximo que deve estar carregado (inclusive). */
int32_t StreamingController_GetMaxChunkZ(const StreamingController* sc);

#endif /* STREAMING_CONTROLLER_H */
