#ifndef ARC_TERMINAL_SCREEN_H
#define ARC_TERMINAL_SCREEN_H

#include <raylib.h>
#include <stdbool.h>

/* ============================================================================
 * ARC Terminal Screen — Tela do terminal ARC (estilo terminal-with-raylib)
 * Renderiza em RenderTexture para exibição em 3D (ex: face de pilar na nave).
 * Não altera código existente; apenas adiciona à gameplay.
 * ============================================================================ */

typedef struct ArcTerminalScreen ArcTerminalScreen;

/* Cria e inicializa o módulo. w/h = resolução da tela do terminal. */
ArcTerminalScreen* ArcTerminalScreen_Create(int width, int height);

/* Libera recursos. */
void ArcTerminalScreen_Destroy(ArcTerminalScreen* arc);

/* Atualiza timers internos (chamar a cada frame). */
void ArcTerminalScreen_Update(ArcTerminalScreen* arc, float dt);

/* Renderiza o conteúdo do terminal no RenderTexture interno. */
void ArcTerminalScreen_RenderToTexture(ArcTerminalScreen* arc);

/* Retorna a textura renderizada (para desenhar em 3D). */
Texture2D ArcTerminalScreen_GetTexture(const ArcTerminalScreen* arc);

/* Retorna true se inicializado corretamente. */
bool ArcTerminalScreen_IsValid(const ArcTerminalScreen* arc);

#endif /* ARC_TERMINAL_SCREEN_H */
