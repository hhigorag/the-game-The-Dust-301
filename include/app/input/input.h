#ifndef INPUT_H
#define INPUT_H

#include "bindings.h"
#include "../../core/gameplay/player.h"

// Atualiza o sistema de input (chamado todo frame)
void Input_Update(void);

// Retorna o comando de input atual
InputCmd Input_GetCommand(void);

// Verifica se uma tecla está pressionada
bool Input_IsKeyPressed(int key);

// Verifica se uma tecla foi pressionada neste frame
bool Input_IsKeyJustPressed(int key);

#endif // INPUT_H
