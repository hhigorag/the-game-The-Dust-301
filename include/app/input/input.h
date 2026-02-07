#ifndef INPUT_H
#define INPUT_H

#include "bindings.h"
#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// INPUT COMMAND (para compatibilidade com sistema antigo)
// ============================================================================

typedef struct {
    float moveX;  // Movimento horizontal (-1 a +1)
    float moveY;  // Movimento vertical (-1 a +1)
    bool interact; // Interação
    uint32_t buttons; // Flags de botões (para compatibilidade)
} InputCmd;

// Atualiza o sistema de input (chamado todo frame)
void Input_Update(void);

// Retorna o comando de input atual
InputCmd Input_GetCommand(void);

// Verifica se uma tecla está pressionada
bool Input_IsKeyPressed(int key);

// Verifica se uma tecla foi pressionada neste frame
bool Input_IsKeyJustPressed(int key);

#endif // INPUT_H
