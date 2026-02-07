#include "app/scenes/scene_menu.h"
#include "app/scenes/scene_manager.h"
#include "core/net/net.h"
#include "core/core.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <raylib.h>
#include <raymath.h>

// Função auxiliar para encontrar caminho correto do asset
static const char* GetAssetPath(const char* relativePath) {
    static char paths[3][512];
    
    // Tenta caminho relativo (executável em build/)
    if (FileExists(relativePath)) {
        return relativePath;
    }
    
    // Tenta caminho relativo ao diretório pai
    snprintf(paths[0], sizeof(paths[0]), "../%s", relativePath);
    if (FileExists(paths[0])) {
        return paths[0];
    }
    
    // Tenta caminho absoluto
    const char* appDir = GetApplicationDirectory();
    snprintf(paths[1], sizeof(paths[1]), "%s%s", appDir, relativePath);
    // Normaliza barras para Windows
    for (char* p = paths[1]; *p; p++) {
        if (*p == '/') *p = '\\';
    }
    if (FileExists(paths[1])) {
        return paths[1];
    }
    
    // Retorna caminho relativo como fallback
    return relativePath;
}

// Opções do menu principal
typedef enum {
    OPTION_HOST_GAME = 0,
    OPTION_JOIN_LOCALHOST = 1,
    OPTION_QUIT = 2,
    OPTION_COUNT = 3
} MenuOption;

// Variáveis globais
static bool g_initialized = false;
static int selectedIndex = 0;  // Índice da opção selecionada (0-2)
static float menuTime = 0.0f;
static float mouseParallaxX = 0.0f;
static float mouseParallaxY = 0.0f;
static float optionGlow[OPTION_COUNT] = {0};
static Shader crtShader = {0};
static RenderTexture2D targetBg = {0};  /* só o fundo; CRT aplicado a esse layer, título/opções por cima */
static Texture2D backgroundTexture = {0};
static Texture2D titleTexture = {0};  /* TITLE.png sobre o fundo, sem parallax */
static Font consolaFont = {0};
static float blinkingRectTime = 0.0f;

// Nomes das opções
static const char* optionNames[] = {
    "HOST GAME",
    "JOIN LOCALHOST",
    "QUIT"
};

// Descrições das opções
static const char* optionDescriptions[] = {
    "Host - create a new game session on port 27015.",
    "Client - join an existing game session at localhost:27015.",
    "Quit - exit the game."
};

/* Layout: título no canto direito; opções na parte inferior da tela, à esquerda */
#define TITLE_INSET_RIGHT   48
#define TITLE_INSET_TOP     36
#define TITLE_MAX_WIDTH_PCT 0.35f   /* título ocupa no máx. 35% da largura */
#define MENU_PANEL_INSET_X  60      /* opções à esquerda */
#define MENU_BOTTOM_MARGIN  96      /* margem da borda inferior (não colado na tela) */
#define MENU_OPTION_PADDING_LEFT  0
#define MENU_OPTION_PADDING_TOP   4
#define MENU_OPTION_LINE_HEIGHT   24   /* altura por linha */
#define MENU_FONT_SIZE       18     /* letras um pouco maiores */

/* Retorna o Y (topo) do bloco de opções — parte inferior da tela, com margem da borda */
static int GetMenuPanelStartY(void) {
    int height = GetScreenHeight();
    float blockH = (float)(OPTION_COUNT * MENU_OPTION_LINE_HEIGHT) + (float)MENU_OPTION_PADDING_TOP * 2.0f;
    return (int)((float)height - blockH - (float)MENU_BOTTOM_MARGIN);
}

// Funções auxiliares
static void UpdateParallax(void) {
    Vector2 mousePos = GetMousePosition();
    Vector2 screenCenter = {(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2};
    
    mouseParallaxX = (mousePos.x - screenCenter.x) / screenCenter.x * 0.05f;
    mouseParallaxY = (mousePos.y - screenCenter.y) / screenCenter.y * 0.05f;
}

static void DrawBackground(void) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    
    if (backgroundTexture.id == 0) {
        DrawRectangle(0, 0, width, height, BLACK);
        return;
    }
    
    float offsetX = mouseParallaxX * 20.0f;
    float offsetY = mouseParallaxY * 20.0f;
    
    float scaleX = (float)width / (float)backgroundTexture.width;
    float scaleY = (float)height / (float)backgroundTexture.height;
    float scale = (scaleX > scaleY) ? scaleX : scaleY;
    
    DrawTextureEx(backgroundTexture, 
                  (Vector2){width/2.0f - (backgroundTexture.width * scale)/2.0f + offsetX, 
                           height/2.0f - (backgroundTexture.height * scale)/2.0f + offsetY}, 
                  0.0f, scale, WHITE);
}

/* Título (TITLE.png) alinhado ao canto direito da tela, sem parallax */
static void DrawTitle(void) {
    if (titleTexture.id == 0) return;
    int width = GetScreenWidth();
    float maxW = (float)width * TITLE_MAX_WIDTH_PCT;
    float scale = maxW / (float)titleTexture.width;
    float w = (float)titleTexture.width * scale;
    float x = (float)width - (float)TITLE_INSET_RIGHT - w;
    DrawTextureEx(titleTexture, (Vector2){x, (float)TITLE_INSET_TOP}, 0.0f, scale, WHITE);
}

static void DrawVignette(void) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float maxDist = sqrtf(centerX * centerX + centerY * centerY);
    
    for (int y = 0; y < height; y += 8) {
        for (int x = 0; x < width; x += 8) {
            float dist = sqrtf((x - centerX) * (x - centerX) + (y - centerY) * (y - centerY));
            float alpha = powf(dist / maxDist, 2.0f) * 0.8f;
            if (alpha > 0.05f) {
                DrawRectangle(x, y, 8, 8, (Color){0, 0, 0, (unsigned char)(alpha * 255)});
            }
        }
    }
}

static void DrawHorizontalBar(void) {
    int width = GetScreenWidth();
    float blockTop = (float)GetMenuPanelStartY();
    float totalHeight = (float)(OPTION_COUNT * MENU_OPTION_LINE_HEIGHT) + (float)MENU_OPTION_PADDING_TOP * 2.0f;
    float barY = blockTop + totalHeight + 16.0f;
    
    float barHeight = 2.0f;
    float margin = 100.0f;
    float fadeWidth = 150.0f;
    float barStartX = margin;
    float barEndX = (float)width - margin;
    float barWidth = barEndX - barStartX;
    
    Color barColor = (Color){0, 255, 150, 200};
    
    int segments = (int)barWidth;
    for (int i = 0; i < segments; i++) {
        float x = barStartX + (float)i;
        float distFromLeft = x - barStartX;
        float distFromRight = barEndX - x;
        float minDist = fminf(distFromLeft, distFromRight);
        
        float alpha = 1.0f;
        if (minDist < fadeWidth) {
            alpha = minDist / fadeWidth;
        }
        
        if (x <= barStartX + 1.0f || x >= barEndX - 1.0f) {
            alpha = 0.0f;
        }
        
        Color segmentColor = (Color){
            barColor.r,
            barColor.g,
            barColor.b,
            (unsigned char)(barColor.a * alpha)
        };
        
        DrawRectangle((int)x, (int)barY, 1, (int)barHeight, segmentColor);
    }
    
    float belowBarY = barY + barHeight + 12.0f;
    float smallFontSize = 16.0f;   /* descrição e versão, próximo do tamanho das opções */
    
    const char* desc = optionDescriptions[selectedIndex];
    DrawTextEx(consolaFont, desc, (Vector2){barStartX, belowBarY}, smallFontSize, 0.0f, WHITE);
    
    const char* versionText = "alpha v0.1";
    Vector2 versionSize = MeasureTextEx(consolaFont, versionText, smallFontSize, 0.0f);
    float versionX = (float)width - margin - versionSize.x;
    DrawTextEx(consolaFont, versionText, (Vector2){versionX, belowBarY}, smallFontSize, 0.0f, WHITE);
}

static void DrawMenuOptions(void) {
    int startX = MENU_PANEL_INSET_X + MENU_OPTION_PADDING_LEFT;
    int startY = GetMenuPanelStartY() + MENU_OPTION_PADDING_TOP;
    int fontSize = MENU_FONT_SIZE;
    int lineH = MENU_OPTION_LINE_HEIGHT;
    
    for (int i = 0; i < OPTION_COUNT; i++) {
        int y = startY + i * lineH;
        float glow = optionGlow[i];
        Color textColor = (Color){
            200 + (unsigned char)(glow * 55),
            200 + (unsigned char)(glow * 55),
            205 + (unsigned char)(glow * 50),
            255
        };
        DrawTextEx(consolaFont, optionNames[i], (Vector2){(float)startX, (float)y}, (float)fontSize, 0.0f, textColor);
    }
    
    DrawHorizontalBar();
}

static void UpdateMenuInteractions(void) {
    Vector2 mousePos = GetMousePosition();
    int startX = MENU_PANEL_INSET_X + MENU_OPTION_PADDING_LEFT;
    int startY = GetMenuPanelStartY() + MENU_OPTION_PADDING_TOP;
    float maxTextW = 0.0f;
    for (int i = 0; i < OPTION_COUNT; i++) {
        Vector2 sz = MeasureTextEx(consolaFont, optionNames[i], (float)MENU_FONT_SIZE, 0.0f);
        if (sz.x > maxTextW) maxTextW = sz.x;
    }
    int lineH = MENU_OPTION_LINE_HEIGHT;
    
    int hoveredOption = -1;
    for (int i = 0; i < OPTION_COUNT; i++) {
        int y = startY + i * lineH;
        Rectangle optionRect = { (float)startX, (float)y, maxTextW + 24.0f, (float)lineH };
        if (CheckCollisionPointRec(mousePos, optionRect)) {
            hoveredOption = i;
            selectedIndex = i;
            break;
        }
    }
    
    // Teclado: navegação com setas/WASD
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        selectedIndex = (selectedIndex - 1 + OPTION_COUNT) % OPTION_COUNT;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        selectedIndex = (selectedIndex + 1) % OPTION_COUNT;
    }
    
    // Atualiza glow baseado em selectedIndex e hover
    for (int i = 0; i < OPTION_COUNT; i++) {
        if (i == selectedIndex && (hoveredOption == i || hoveredOption == -1)) {
            // Aumenta glow se está selecionado (por teclado ou mouse)
            optionGlow[i] = fminf(optionGlow[i] + GetFrameTime() * 3.0f, 1.0f);
        } else {
            // Diminui glow se não está selecionado
            optionGlow[i] = fmaxf(optionGlow[i] - GetFrameTime() * 3.0f, 0.0f);
        }
    }
    
    // ENTER: executa ação da opção selecionada; Click: só se o mouse estiver em cima do botão
    int optionToRun = -1;
    if (IsKeyPressed(KEY_ENTER)) {
        optionToRun = selectedIndex;
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoveredOption >= 0) {
        optionToRun = hoveredOption;
    }
    if (optionToRun >= 0 && optionToRun < OPTION_COUNT) {
        NetSystem* net = Core_GetNetSystem();
        switch (optionToRun) {
                case OPTION_HOST_GAME: {
                    // HOST GAME: Net_StartHost(27015)
                    NetConfig cfg = {0};
                    cfg.port = 27015;
                    if (Net_StartHost(net, &cfg, 8)) {
                        printf("[MENU] Host iniciado na porta 27015\n");
                        SceneManager_Change(SCENE_GAMEPLAY);
                    } else {
                        printf("[MENU] Erro ao iniciar host\n");
                    }
                    break;
                }
                case OPTION_JOIN_LOCALHOST: {
                    // JOIN LOCALHOST: Net_Connect("127.0.0.1", 27015)
                    NetConfig cfg = {0};
                    cfg.connect_ip = "127.0.0.1";
                    cfg.port = 27015;
                    if (Net_Connect(net, &cfg)) {
                        printf("[MENU] Conectando a 127.0.0.1:27015...\n");
                        SceneManager_Change(SCENE_GAMEPLAY);
                    } else {
                        printf("[MENU] Erro ao conectar\n");
                    }
                    break;
                }
                case OPTION_QUIT: {
                    CloseWindow();
                    break;
                }
        }
    }
    
    // ESC: não faz nada no menu principal (conforme especificação)
    // (ou pode sair do jogo se quiser, mas a especificação diz "opcional")
}

void Scene_Menu_Shutdown(void) {
    if (!g_initialized) return;
    if (targetBg.id != 0) UnloadRenderTexture(targetBg);
    targetBg = (RenderTexture2D){0};
    if (crtShader.id != 0) { UnloadShader(crtShader); crtShader.id = 0; }
    if (backgroundTexture.id != 0) { UnloadTexture(backgroundTexture); backgroundTexture = (Texture2D){0}; }
    if (titleTexture.id != 0) { UnloadTexture(titleTexture); titleTexture = (Texture2D){0}; }
    g_initialized = false;
}

void Scene_Menu_Init(void) {
    if (g_initialized) return;
    
    srand((unsigned)((int)(GetTime() * 1000.0f)));
    
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    targetBg = LoadRenderTexture(width, height);
    
    // Carrega shader (CRT só no layer do fundo; título e opções por cima)
    const char* shaderPath = GetAssetPath("assets/ui/menu-crt.fs");
    crtShader = LoadShader(0, shaderPath);
    if (crtShader.id == 0) {
        TraceLog(LOG_WARNING, "Falha ao carregar shader menu-crt.fs");
    }
    
    // Carrega textura de fundo
    const char* bgPath = GetAssetPath("assets/ui/fundo.png");
    backgroundTexture = LoadTexture(bgPath);
    if (backgroundTexture.id == 0) {
        TraceLog(LOG_ERROR, "Falha ao carregar textura de fundo");
    }
    
    // Carrega título (sobre o fundo, sem parallax)
    const char* titlePath = GetAssetPath("assets/ui/TITLE.png");
    titleTexture = LoadTexture(titlePath);
    if (titleTexture.id == 0) {
        TraceLog(LOG_WARNING, "Falha ao carregar TITLE.png");
    }
    
    // Carrega fonte (assets/fonts)
    const char* fontPath = GetAssetPath("assets/fonts/CONSOLA.TTF");
    consolaFont = LoadFont(fontPath);
    if (consolaFont.texture.id == 0) {
        const char* fontPathLower = GetAssetPath("assets/fonts/CONSOLA.ttf");
        consolaFont = LoadFont(fontPathLower);
    }
    if (consolaFont.texture.id == 0) {
        TraceLog(LOG_WARNING, "Falha ao carregar fonte CONSOLA! Usando fonte padrão.");
        consolaFont = GetFontDefault();
    } else {
        SetTextureFilter(consolaFont.texture, TEXTURE_FILTER_BILINEAR);
    }
    
    selectedIndex = 0;
    EnableCursor();
    g_initialized = true;
}

void Scene_Menu_Update(float dt) {
    (void)dt;
    
    menuTime += GetFrameTime();
    blinkingRectTime += GetFrameTime();
    
    UpdateParallax();
    UpdateMenuInteractions();
}

void Scene_Menu_Draw(void) {
    Rectangle srcRect = { 0, 0, (float)targetBg.texture.width, -(float)targetBg.texture.height };

    /* 1) Só o fundo no RT; CRT será aplicado só a esse layer */
    BeginTextureMode(targetBg);
    ClearBackground(BLACK);
    DrawBackground();
    EndTextureMode();

    /* 2) Fundo com CRT (fullscreen), depois título uma vez, vignette e opções por cima — sem região retangular no meio */
    BeginDrawing();
    ClearBackground(BLACK);
    if (crtShader.id != 0) {
        BeginShaderMode(crtShader);
        int timeLoc = GetShaderLocation(crtShader, "time");
        if (timeLoc >= 0) {
            SetShaderValue(crtShader, timeLoc, &menuTime, SHADER_UNIFORM_FLOAT);
        }
        DrawTextureRec(targetBg.texture, srcRect, (Vector2){0, 0}, WHITE);
        EndShaderMode();
    } else {
        DrawTextureRec(targetBg.texture, srcRect, (Vector2){0, 0}, WHITE);
    }
    DrawTitle();
    DrawVignette();
    DrawMenuOptions();
    EndDrawing();
}
