/*
 * ARC Terminal Screen — Tela estilo terminal ARC Shell (inspirado em terminal-with-raylib).
 * Renderiza em RenderTexture para exibição em face de pilar 3D na nave.
 */
#include "app/ui/arc_terminal_screen.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

#define ARC_COLOR_GREEN ((Color){51, 255, 51, 255})
#define MAX_DISPLAY_LINES 12

struct ArcTerminalScreen {
    RenderTexture2D target;
    Font font;
    Texture2D logo;
    bool fontLoaded;
    bool logoLoaded;
    float animTimer;
    int lineCount;
    char lines[MAX_DISPLAY_LINES][128];
};

static void DrawShellTextStyle(Font font, const char* text, float x, float y, float size, Color color) {
    if (!text) return;
    Vector2 pos = { x, y };
    DrawTextEx(font, text, pos, size, 2.0f, color);
}

static const char* GetAssetPath_Arc(const char* rel) {
    if (FileExists(rel)) return rel;
    static char b[2][512];
    snprintf(b[0], sizeof(b[0]), "../%s", rel);
    if (FileExists(b[0])) return b[0];
    const char* appDir = GetApplicationDirectory();
    snprintf(b[1], sizeof(b[1]), "%s%s", appDir, rel);
    for (char* p = b[1]; *p; p++) { if (*p == '/') *p = '\\'; }
    if (FileExists(b[1])) return b[1];
    return rel;
}

ArcTerminalScreen* ArcTerminalScreen_Create(int width, int height) {
    if (width < 64 || height < 64) return NULL;

    ArcTerminalScreen* arc = (ArcTerminalScreen*)calloc(1, sizeof(ArcTerminalScreen));
    if (!arc) return NULL;

    arc->target = LoadRenderTexture(width, height);
    if (!arc->target.id || !IsRenderTextureValid(arc->target)) {
        free(arc);
        return NULL;
    }

    const char* fontPath = GetAssetPath_Arc("assets/fonts/CONSOLA.TTF");
    if (!FileExists(fontPath))
        fontPath = GetAssetPath_Arc("assets/fonts/CONSOLA.ttf");
    arc->font = LoadFontEx(fontPath, 32, 0, 400);
    arc->fontLoaded = (arc->font.texture.id != 0);
    if (arc->fontLoaded)
        SetTextureFilter(arc->font.texture, TEXTURE_FILTER_BILINEAR);
    else
        arc->font = GetFontDefault();

    const char* logoPath = GetAssetPath_Arc("assets/ui/ARC_logo.png");
    if (FileExists(logoPath)) {
        arc->logo = LoadTexture(logoPath);
        arc->logoLoaded = (arc->logo.id != 0);
    } else {
        arc->logoLoaded = false;
    }

    arc->animTimer = 0.0f;
    arc->lineCount = 0;
    arc->lines[0][0] = '\0';

    return arc;
}

void ArcTerminalScreen_Destroy(ArcTerminalScreen* arc) {
    if (!arc) return;
    if (arc->target.id) UnloadRenderTexture(arc->target);
    if (arc->fontLoaded && arc->font.texture.id) UnloadFont(arc->font);
    if (arc->logoLoaded && arc->logo.id) UnloadTexture(arc->logo);
    free(arc);
}

void ArcTerminalScreen_Update(ArcTerminalScreen* arc, float dt) {
    if (!arc) return;
    arc->animTimer += dt;
}

void ArcTerminalScreen_RenderToTexture(ArcTerminalScreen* arc) {
    if (!arc || !arc->target.id) return;

    float w = (float)arc->target.texture.width;
    float h = (float)arc->target.texture.height;

    BeginTextureMode(arc->target);
    ClearBackground(BLACK);

    /* Marca d'água do logo */
    if (arc->logoLoaded) {
        float scale = 0.35f;
        float lw = (float)arc->logo.width * scale;
        float lh = (float)arc->logo.height * scale;
        float lx = (w - lw) * 0.5f;
        float ly = (h - lh) * 0.5f;
        DrawTextureEx(arc->logo, (Vector2){lx, ly}, 0.0f, scale, (Color){255, 255, 255, 25});
    }

    /* Borda verde */
    DrawRectangleLinesEx((Rectangle){8, 8, w - 16, h - 16}, 2, ARC_COLOR_GREEN);

    /* Cabeçalho */
    const char* header = "ARC_SHELL | [NAVIGATION MODE]";
    Vector2 tw = MeasureTextEx(arc->font, header, 14, 2);
    DrawShellTextStyle(arc->font, header, (w - tw.x) * 0.5f, 12, 14, ARC_COLOR_GREEN);

    /* Data/hora: usa GetTime() para evitar dependência de time.h (conflitos em alguns builds). */
    {
        float t = (float)GetTime();
        int min = ((int)t / 60) % 60;
        int hour = ((int)t / 3600) % 24;
        char buf[32];
        snprintf(buf, sizeof(buf), "%02d:%02d 08/02/2226", hour, min);
        DrawShellTextStyle(arc->font, buf, 14, 14, 11, WHITE);
    }

    /* Linhas de output (simuladas) */
    {
        const char* staticLines[] = {
            "> STATUS: ALL RIGHT",
            "> LOCATION: IN ORBIT",
            "> ENGINE: IDLE",
            "> DOORS: SEALED",
            "> TARGETS: PROXIMA CENTAURI B",
            "> RISK LEVEL: [1]",
            "> ANOMALIES: NONE"
        };
        int n = (int)(sizeof(staticLines) / sizeof(staticLines[0]));
        float lineY = 38;
        float lineH = 14;
        for (int i = 0; i < n && lineY + lineH < h - 36; i++) {
            DrawShellTextStyle(arc->font, staticLines[i], 14, lineY, 11, ARC_COLOR_GREEN);
            lineY += lineH;
        }
    }

    /* Linha do prompt */
    float promptY = h - 28;
    DrawLineEx((Vector2){12, promptY - 6}, (Vector2){w - 12, promptY - 6}, 1, ARC_COLOR_GREEN);
    DrawShellTextStyle(arc->font, "operator-301@ARC_Shell>", 14, promptY, 12, ARC_COLOR_GREEN);

    /* Cursor piscante */
    if ((int)(arc->animTimer * 2.5f) % 2 == 0) {
        float cw = MeasureTextEx(arc->font, "operator-301@ARC_Shell>", 12, 2).x;
        DrawRectangle(14 + (int)cw + 4, (int)promptY + 2, 10, 14, ARC_COLOR_GREEN);
    }

    EndTextureMode();
}

Texture2D ArcTerminalScreen_GetTexture(const ArcTerminalScreen* arc) {
    if (!arc) return (Texture2D){0};
    return arc->target.texture;
}

bool ArcTerminalScreen_IsValid(const ArcTerminalScreen* arc) {
    return arc && arc->target.id != 0 && IsRenderTextureValid(arc->target);
}
