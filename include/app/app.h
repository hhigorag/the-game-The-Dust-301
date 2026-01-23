#ifndef APP_H
#define APP_H

#include <stdbool.h>

// Inicializa o App (janela, áudio, etc.)
int App_Init(int width, int height, const char* title);

// Finaliza o App
void App_Shutdown(void);

// Verifica se a janela deve fechar
bool App_ShouldClose(void);

// Verifica se a janela está inicializada
bool App_IsInitialized(void);

#endif // APP_H
