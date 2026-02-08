#ifndef ARC_TERMINAL_FULL_H
#define ARC_TERMINAL_FULL_H

#include <raylib.h>
#include <stdbool.h>

/* Terminal ARC completo: boot → auth → loading → shell.
 * Overlay 1600x920 centralizado; toggle com E. */

typedef struct ArcTerminalFull ArcTerminalFull;

ArcTerminalFull* ArcTerminalFull_Create(void);
void ArcTerminalFull_Destroy(ArcTerminalFull* t);

void ArcTerminalFull_Open(ArcTerminalFull* t);
void ArcTerminalFull_Close(ArcTerminalFull* t);
bool ArcTerminalFull_IsOpen(const ArcTerminalFull* t);
bool ArcTerminalFull_IsInShell(const ArcTerminalFull* t);

void ArcTerminalFull_Update(ArcTerminalFull* t, float dt);
void ArcTerminalFull_ProcessKey(ArcTerminalFull* t, int key);
void ArcTerminalFull_Render(ArcTerminalFull* t);

/* Retorna textura 1600x920 para desenhar centralizada. */
Texture2D ArcTerminalFull_GetTexture(const ArcTerminalFull* t);

/* Posição/dimensões do overlay (para centralizar em 1920x1080). */
void ArcTerminalFull_GetOverlayRect(int screenW, int screenH, Rectangle* outRect);

#endif
