/*
 * ARC Terminal Full — Terminal completo (boot → auth → loading → shell).
 * Reimportado de terminal-with-raylib. Overlay 1600x920 centralizado; toggle com E.
 */
#include "app/ui/arc_terminal_full.h"
#include "app/ui/arc_terminal/arc_common.h"
#include "app/ui/arc_terminal/arc_utils.h"
#include "app/ui/arc_terminal/arc_render.h"
#include "app/ui/arc_terminal/boot.h"
#include "app/ui/arc_terminal/login.h"
#include "app/ui/arc_terminal/shell.h"
#include "app/ui/arc_clim.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <raylib.h>

#define ARC_W 1600
#define ARC_H 920

static const char* GetAssetPath(const char* rel) {
    if (FileExists(rel)) return rel;
    static char b[512];
    snprintf(b, sizeof(b), "../%s", rel);
    if (FileExists(b)) return b;
    const char* base = GetApplicationDirectory();
    if (base) {
        snprintf(b, sizeof(b), "%s%s", base, rel);
        for (char* p = b; *p; p++) if (*p == '/') *p = '\\';
        if (FileExists(b)) return b;
    }
    return rel;
}

struct ArcTerminalFull {
    bool open;
    RenderTexture2D target;
    Font font;
    Texture2D logo;
    bool fontOk;
    bool logoOk;
    ArcTerminalState state;
    char username[32];
    int loginLen;
    bool authError;
    float bootTimer;
    float blackTimer;
    float loadProgress;
    float loadVisProgress;
    float loadTimer;
    float loadPauseTimer;
    bool loadPaused;
    float nextPause;
    float loadTotal;
    float loadCompleteTimer;
    bool loadDone;
    float loadDotsTimer;
    char cpuName[256];
    unsigned long long totalRam;
    ArcShellContext* shell;
};

ArcTerminalFull* ArcTerminalFull_Create(void) {
    ArcTerminalFull* t = (ArcTerminalFull*)calloc(1, sizeof(ArcTerminalFull));
    if (!t) return NULL;

    t->target = LoadRenderTexture(ARC_W, ARC_H);
    if (!t->target.id || !IsRenderTextureValid(t->target)) {
        free(t);
        return NULL;
    }

    arc_get_cpu_name(t->cpuName);
    t->totalRam = arc_get_total_ram();
    if (t->totalRam == 0) t->totalRam = 16;

    const char* fp = GetAssetPath("assets/fonts/CONSOLA.TTF");
    if (!FileExists(fp)) fp = GetAssetPath("assets/fonts/CONSOLA.ttf");
    t->font = LoadFontEx(fp, 28, 0, 256);
    t->fontOk = (t->font.texture.id != 0);
    if (t->fontOk) SetTextureFilter(t->font.texture, TEXTURE_FILTER_BILINEAR);
    else t->font = GetFontDefault();

    const char* lp = GetAssetPath("assets/ui/ARC_logo.png");
    if (FileExists(lp)) {
        t->logo = LoadTexture(lp);
        t->logoOk = (t->logo.id != 0);
    } else {
        t->logoOk = false;
    }

    InitClimate();
    t->shell = Arc_Shell_Create();
    if (!t->shell) {
        if (t->target.id) UnloadRenderTexture(t->target);
        if (t->fontOk && t->font.texture.id) UnloadFont(t->font);
        if (t->logoOk && t->logo.id) UnloadTexture(t->logo);
        free(t);
        return NULL;
    }

    return t;
}

void ArcTerminalFull_Destroy(ArcTerminalFull* t) {
    if (!t) return;
    if (t->shell) Arc_Shell_Destroy(t->shell);
    if (t->target.id) UnloadRenderTexture(t->target);
    if (t->fontOk && t->font.texture.id) UnloadFont(t->font);
    if (t->logoOk && t->logo.id) UnloadTexture(t->logo);
    free(t);
}

void ArcTerminalFull_Open(ArcTerminalFull* t) {
    if (!t) return;
    t->open = true;
    t->state = ARC_STATE_BOOT;
    t->bootTimer = 0;
    t->loginLen = 0;
    t->username[0] = '\0';
    t->authError = false;
    t->blackTimer = 0;
    t->loadProgress = 0;
    t->loadVisProgress = 0;
    t->loadTimer = 0;
    t->loadPauseTimer = 0;
    t->loadPaused = false;
    t->nextPause = 0.3f + (float)(GetRandomValue(0, 70)) / 100.0f;
    t->loadTotal = 0;
    t->loadCompleteTimer = 0;
    t->loadDone = false;
    t->loadDotsTimer = 0;
    Arc_Shell_Reset(t->shell, t->username);
}

void ArcTerminalFull_Close(ArcTerminalFull* t) {
    if (t) t->open = false;
}

bool ArcTerminalFull_IsOpen(const ArcTerminalFull* t) {
    return t && t->open;
}

static int norm_enter(int key) {
    return (key == 10 || key == 13 || key == 257) ? KEY_ENTER : key;
}

void ArcTerminalFull_ProcessKey(ArcTerminalFull* t, int key) {
    if (!t || !t->open) return;
    key = norm_enter(key);

    if (t->state == ARC_STATE_LOGIN) {
        ArcLoginLogicContext ctx = {
            t->username, &t->loginLen, &t->authError
        };
        if (Arc_Login_ProcessKey(&ctx, key)) {
            t->state = ARC_STATE_LOADING;
            t->blackTimer = 0;
            t->loadProgress = 0;
            t->loadVisProgress = 0;
            t->loadTimer = 0;
            t->loadPaused = false;
            t->loadTotal = 0;
            t->loadCompleteTimer = 0;
            t->loadDone = false;
            t->loadDotsTimer = 0;
            t->nextPause = 0.3f + (float)(GetRandomValue(0, 70)) / 100.0f;
        }
        return;
    }

    if (t->state == ARC_STATE_SHELL) {
        Arc_Shell_ProcessKey(t->shell, key, t->username, t->cpuName, t->totalRam, ARC_W, ARC_H);
    }
}

void ArcTerminalFull_Update(ArcTerminalFull* t, float dt) {
    if (!t || !t->open) return;

    if (t->state == ARC_STATE_BOOT) {
        Arc_Boot_Update(&t->bootTimer, &t->state, dt);
        return;
    }

    if (t->state == ARC_STATE_LOADING) {
        t->loadTotal += dt;
        t->loadDotsTimer += dt;
        if (t->loadDotsTimer >= 1.0f) t->loadDotsTimer = 0;

        if (t->loadDone) {
            t->loadCompleteTimer += dt;
            if (t->loadCompleteTimer >= 2.0f) {
                t->state = ARC_STATE_SHELL;
                Arc_Shell_Reset(t->shell, t->username);
            }
            return;
        }

        t->blackTimer += dt;
        if (t->blackTimer < 3.0f) return;

        float elapsed = t->loadTotal - 3.0f;
        t->loadProgress = elapsed / 4.0f;
        if (t->loadProgress > 1.0f) {
            t->loadProgress = 1.0f;
            t->loadDone = true;
            t->loadCompleteTimer = 0;
        }
        if (t->loadProgress >= 1.0f) t->loadVisProgress = 1.0f;

        if (t->loadPaused) {
            t->loadPauseTimer += dt;
            if (t->loadPauseTimer >= 0.2f + (float)(GetRandomValue(0, 60)) / 100.0f) {
                t->loadPaused = false;
                t->loadPauseTimer = 0;
                t->nextPause = 0.3f + (float)(GetRandomValue(0, 90)) / 100.0f;
                t->loadTimer = 0;
            }
        } else {
            t->loadTimer += dt;
            if (t->loadTimer >= t->nextPause) {
                t->loadPaused = true;
                t->loadPauseTimer = 0;
            } else {
                t->loadVisProgress += dt / 4.0f;
                if (t->loadVisProgress > t->loadProgress) t->loadVisProgress = t->loadProgress;
            }
        }
        return;
    }

    if (t->state == ARC_STATE_SHELL) {
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        int ovX = (screenW - ARC_W) / 2;
        int ovY = (screenH - ARC_H) / 2;
        if (ovX < 0) ovX = 0;
        if (ovY < 0) ovY = 0;

        Vector2 mouse = GetMousePosition();
        int mouseLocalX = (int)mouse.x - ovX;
        int mouseLocalY = (int)mouse.y - ovY;
        bool inOverlay = (mouseLocalX >= 0 && mouseLocalX < ARC_W && mouseLocalY >= 0 && mouseLocalY < ARC_H);

        Arc_Shell_Update(t->shell, (float)ARC_W, (float)ARC_H,
            inOverlay ? mouseLocalX : -1,
            inOverlay ? mouseLocalY : -1,
            inOverlay && IsMouseButtonPressed(MOUSE_BUTTON_LEFT),
            GetMouseWheelMove(), dt);
    }
}

void ArcTerminalFull_Render(ArcTerminalFull* t) {
    if (!t || !t->open || !t->target.id) return;

    BeginTextureMode(t->target);
    ClearBackground(BLACK);

    if (t->state == ARC_STATE_BOOT) {
        Arc_Boot_Render(t->bootTimer, t->cpuName, t->totalRam, t->font);
    } else if (t->state == ARC_STATE_LOGIN) {
        ArcLoginRenderContext ctx = {
            t->username, t->loginLen, t->authError,
            (float)ARC_W, (float)ARC_H
        };
        Arc_Login_Render(&ctx, t->font);
    } else if (t->state == ARC_STATE_LOADING) {
        if (t->blackTimer >= 3.0f) {
            float logoScale = 0.5f;
            float lw = (float)t->logo.width * logoScale;
            float lh = (float)t->logo.height * logoScale;
            float lx = (ARC_W - lw) / 2;
            float ly = ARC_H / 2 - 280;
            if (t->logoOk) DrawTextureEx(t->logo, (Vector2){lx, ly}, 0, logoScale, WHITE);

            float barW = 600, barH = 30;
            float barX = (ARC_W - barW) / 2;
            float barY = ly + lh + 40;
            DrawRectangleRec((Rectangle){barX, barY, barW, barH}, (Color){30, 30, 30, 255});
            DrawRectangleLinesEx((Rectangle){barX, barY, barW, barH}, 1, ARC_COLOR_GREEN);
            float pw = barW * t->loadVisProgress;
            if (pw > 0) DrawRectangleRec((Rectangle){barX, barY, pw, barH}, ARC_COLOR_GREEN);

            char loadTxt[16] = "LOADING";
            int dc = (int)(t->loadDotsTimer * 4) % 4;
            for (int i = 0; i < dc && i < 3; i++) strcat(loadTxt, ".");
            Vector2 lts = MeasureTextEx(t->font, loadTxt, 18, 1);
            Arc_DrawShellText(t->font, loadTxt, (ARC_W - lts.x) / 2, barY + barH + 20, 18, ARC_COLOR_GREEN);
        }
    } else if (t->state == ARC_STATE_SHELL) {
        Arc_Shell_Render(t->shell, t->font, t->logo, t->logoOk,
            t->username, t->cpuName, t->totalRam,
            (float)ARC_W, (float)ARC_H);
    }

    EndTextureMode();
}

Texture2D ArcTerminalFull_GetTexture(const ArcTerminalFull* t) {
    if (!t) return (Texture2D){0};
    return t->target.texture;
}

void ArcTerminalFull_GetOverlayRect(int screenW, int screenH, Rectangle* outRect) {
    if (!outRect) return;
    int w = 1600, h = 920;
    int x = (screenW - w) / 2;
    int y = (screenH - h) / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    *outRect = (Rectangle){ (float)x, (float)y, (float)w, (float)h };
}
