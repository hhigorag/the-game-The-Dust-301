#include "app/scenes/scene_gameplay.h"
#include "app/scenes/scene_manager.h"
#include "app/settings/settings.h"
#include "core/state/game_state.h"
#include "core/state/match_state.h"
#include "core/net/net.h"
#include "core/net/protocol.h"
#include "core/core.h"
#include "app/input/input.h"
#include "app/camera/fps_camera.h"
#include "core/physics/physics.h"  /* PLAYER_HEIGHT, PLAYER_EYE_HEIGHT */
#include "app/render/atmosphere.h"
#include "app/render/lighting.h"
#include "core/world/world_beware.h"
#include "core/world/voxel_world.h"
#include "core/world/world_config.h"
#include "core/gameplay/ship.h"
#include "app/ui/arc_terminal_full.h"
#include <raylib.h>
#include <raymath.h>
#if defined(USE_RLGL)
#include <rlgl.h>
#endif
#include <stdint.h>
#include <math.h>

/* Pause: overlay em gameplay (ESC). Debug: F1=grid, F2=direção (N/S/L/O), F3=stats, F4=wireframe. */
#include <string.h>
#include <stdio.h>

typedef enum GameplayMode {
    GP_PLAYING = 0,
    GP_PAUSED  = 1,
} GameplayMode;

typedef enum PausePage {
    PAUSE_MAIN = 0,
    PAUSE_SETTINGS,
    PAUSE_KEYBINDS
} PausePage;

typedef enum SettingsItem {
    SET_FOV = 0,
    SET_SENS,
    SET_CROSSHAIR,
    SET_CONTROLS,
    SET_BACK,
    SET_COUNT
} SettingsItem;

static inline float LerpFloat(float a, float b, float t) {
    return a + (b - a) * t;
}
static inline float SmoothTo(float current, float target, float speed, float dt) {
    float t = 1.0f - expf(-speed * dt);
    return LerpFloat(current, target, t);
}

static inline bool KeyDown(ActionId a) {
    int k1 = g_settings.keys[a].primary;
    int k2 = g_settings.keys[a].secondary;
    return (k1 && IsKeyDown(k1)) || (k2 && IsKeyDown(k2));
}
static inline bool KeyPressed(ActionId a) {
    int k1 = g_settings.keys[a].primary;
    int k2 = g_settings.keys[a].secondary;
    return (k1 && IsKeyPressed(k1)) || (k2 && IsKeyPressed(k2));
}

// Mapa debug: usa BlockType de voxel_world.h (BLOCK_AIR, BLOCK_TERRAIN, BLOCK_GRAY, BLOCK_RED, etc.)
// Mapa simples: array 3D fixo para debugging
#define MAP_SIZE_X 50
#define MAP_SIZE_Y 20
#define MAP_SIZE_Z 50
#define MAP_OFFSET_X (MAP_SIZE_X / 2)
#define MAP_OFFSET_Z (MAP_SIZE_Z / 2)
#define FOG_EXP_TARGET 0.9f  /* em fogEnd, fogT = target; density = -ln(1-target)/fogEnd */
/* Top-only collider: colisão X/Z contra hull só quando player está abaixo do topo (evita empurrão lateral em cima). */
#define COLLISION_ABOVE_HULL_EPS  0.05f

static const char* GetAssetPath(const char* rel) {
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

static bool g_initialized = false;
static GameplayMode g_mode = GP_PLAYING;
static PausePage g_pausePage = PAUSE_MAIN;
static int g_pauseSelectionMain = 0;     // 0=Resume, 1=Settings, 2=Exit
static int g_pauseSelectionSettings = 0; // SET_FOV..SET_BACK
static int g_pauseSelectionBinds = 0;    // 0..ACT_COUNT=Back
static bool g_waitingForKey = false;
static ActionId g_bindingAction = ACT_MOVE_FORWARD;
static bool g_dbgShowGrid = true;       // F1
static bool g_showDirection = false;    // F2 = bússola (NORTE/SUL/LESTE/OESTE)
static bool g_dbgShowStats = false;     // F3
static bool g_dbgShowWireframe = false; // F4, default OFF

typedef struct UiSettingsAnim { float fovVis; float sensVis; } UiSettingsAnim;
static UiSettingsAnim g_uiAnim;
static bool g_uiAnimInit = false;
/* Arrastar slider com mouse: SET_FOV, SET_SENS ou -1 quando não está arrastando */
static int g_draggingSlider = -1;
/* Head bob ao andar/correr: fase do seno (decai quando parado) */
static float g_walkBobPhase = 0.0f;

static BlockType g_map[MAP_SIZE_X][MAP_SIZE_Y][MAP_SIZE_Z]; // Mapa debug 3D (BlockType de voxel_world.h)
static FPSCamera g_fpsCamera;
static PhysicsBody g_playerPhysics;
static bool g_firstFrame = true; // Flag para travar mouse na primeira frame da gameplay
static Font g_consolaFont = {0}; // Fonte CONSOLAS para informações de debug
static bool g_noClip = false; // Modo "No clipping" (movimento livre)
static bool g_isColliding = false; // Flag para detectar colisão (chão ou parede)
static Atmosphere g_atmosphere; // Sistema de atmosfera (fog + sky)
static LightingSystem g_lighting; // Sistema de iluminação direcional fake
static Shader g_fogShader = {0}; // Fog no forward (view-space)
static int g_fogLocStart = -1, g_fogLocEnd = -1, g_fogLocColor = -1, g_fogLocType = -1, g_fogLocDensity = -1;
static int g_fogLocHorizon = -1, g_fogLocSky = -1, g_fogLocH0 = -1, g_fogLocHRange = -1;
static Shader g_skyShader = {0}; // Sky gradient por raio (sem depth)
static int g_skyLocBottom = -1, g_skyLocHorizon = -1, g_skyLocTop = -1;

/* Nave 3D: modelo GLB (carregado para uso futuro; placeholder usa Ship_Draw). */
static Model g_shipModel = {0};
static bool g_shipModelLoaded = false;

/* CRT overlay: renderiza gameplay num RT e desenha por cima com shader (capinha sem afetar nada). */
static RenderTexture2D g_crtTarget = {0};
static Shader g_crtShader = {0};
/* CRT do overlay do terminal ARC (crt.fs do terminal-with-raylib). */
static Shader g_terminalCrtShader = {0};
static int g_terminalCrtTimeLoc = -1;

/* Mundo Beware: streaming por faixa Z e corredor; substitui mapa debug quando ativo. */
static VoxelWorld* g_voxelWorld = NULL;
static WorldBeware g_worldBeware;
static bool g_useStreamingWorld = true;
/* Nave placeholder: posição física, pivô do streaming, zona segura (deck). */
static Ship g_ship;
static bool g_standingOnShip = false;  /* true = player em cima da nave; herda delta. */
static bool g_isOnLadder = false;       /* true = player na escada; sobe automaticamente, gravidade desativada. */

/* Terminal ARC: overlay 1600x920 ao pressionar E perto do monitor. */
static ArcTerminalFull* g_arcTerminalFull = NULL;
static bool g_nearArcMonitor = false;
#define MONITOR_INTERACT_DIST 2.5f

/* Colisão: 3 passes (Y, X, Z) para evitar blocos “puxando” o player.
 * Gravidade só no eixo Y. Colidimos com todo bloco sólido (exceto ar). */
// Verifica se há um bloco sólido na posição do mapa (ou do VoxelWorld quando streaming ativo)
static bool IsBlockSolid(int32_t x, int32_t y, int32_t z) {
    if (g_useStreamingWorld && g_voxelWorld) {
        Voxel v = VoxelWorld_GetBlock(g_voxelWorld, x, y, z);
        return v.type != BLOCK_AIR;
    }
    // Mapa debug fixo
    int32_t mapX = x + MAP_OFFSET_X;
    int32_t mapY = y;
    int32_t mapZ = z + MAP_OFFSET_Z;
    if (mapX < 0 || mapX >= MAP_SIZE_X || mapY < 0 || mapY >= MAP_SIZE_Y || mapZ < 0 || mapZ >= MAP_SIZE_Z) {
        return false;
    }
    return g_map[mapX][mapY][mapZ] != BLOCK_AIR;
}


// Obtém a normal de uma face baseado na direção
static Vector3 GetFaceNormal(int faceDir) {
    switch (faceDir) {
        case 0: return (Vector3){-1.0f, 0.0f, 0.0f}; // -X (esquerda)
        case 1: return (Vector3){1.0f, 0.0f, 0.0f};  // +X (direita)
        case 2: return (Vector3){0.0f, -1.0f, 0.0f};  // -Y (chão)
        case 3: return (Vector3){0.0f, 1.0f, 0.0f};  // +Y (teto)
        case 4: return (Vector3){0.0f, 0.0f, -1.0f}; // -Z (traseira)
        case 5: return (Vector3){0.0f, 0.0f, 1.0f};  // +Z (frontal)
        default: return (Vector3){0.0f, 0.0f, 0.0f};
    }
}

// faceDir: 0=-X, 1=+X, 2=-Y, 3=+Y, 4=-Z, 5=+Z. Retorna false se bx,by,bz inválidos (isfinite).
static bool GetBlockFaceVertices(float bx, float by, float bz, int faceDir, Vector3 v[4]) {
    if (!isfinite((double)bx) || !isfinite((double)by) || !isfinite((double)bz)) return false;
    switch (faceDir) {
        case 0: v[0] = (Vector3){bx, by, bz}; v[1] = (Vector3){bx, by, bz + 1.0f}; v[2] = (Vector3){bx, by + 1.0f, bz + 1.0f}; v[3] = (Vector3){bx, by + 1.0f, bz}; return true;
        case 1: v[0] = (Vector3){bx + 1.0f, by, bz}; v[1] = (Vector3){bx + 1.0f, by + 1.0f, bz}; v[2] = (Vector3){bx + 1.0f, by + 1.0f, bz + 1.0f}; v[3] = (Vector3){bx + 1.0f, by, bz + 1.0f}; return true;
        case 2: v[0] = (Vector3){bx, by, bz}; v[1] = (Vector3){bx + 1.0f, by, bz}; v[2] = (Vector3){bx + 1.0f, by, bz + 1.0f}; v[3] = (Vector3){bx, by, bz + 1.0f}; return true;
        case 3: v[0] = (Vector3){bx, by + 1.0f, bz}; v[1] = (Vector3){bx, by + 1.0f, bz + 1.0f}; v[2] = (Vector3){bx + 1.0f, by + 1.0f, bz + 1.0f}; v[3] = (Vector3){bx + 1.0f, by + 1.0f, bz}; return true;
        case 4: v[0] = (Vector3){bx, by, bz}; v[1] = (Vector3){bx, by + 1.0f, bz}; v[2] = (Vector3){bx + 1.0f, by + 1.0f, bz}; v[3] = (Vector3){bx + 1.0f, by, bz}; return true;
        case 5: v[0] = (Vector3){bx, by, bz + 1.0f}; v[1] = (Vector3){bx + 1.0f, by, bz + 1.0f}; v[2] = (Vector3){bx + 1.0f, by + 1.0f, bz + 1.0f}; v[3] = (Vector3){bx, by + 1.0f, bz + 1.0f}; return true;
        default: return false;
    }
}

// Passe 1 — só triângulos (faces sólidas). Fog no shader. pFacesInBatch: flush a cada 256.
static void DrawBlockFace_Solid(float bx, float by, float bz, int faceDir, Color baseColor, int* pFacesInBatch) {
    Vector3 v[4];
    if (!GetBlockFaceVertices(bx, by, bz, faceDir, v)) return;
    Vector3 n = GetFaceNormal(faceDir);
    Color lit = Lighting_ApplyDirectionalLight(&g_lighting, n, baseColor);
    DrawTriangle3D(v[0], v[1], v[2], lit);
    DrawTriangle3D(v[0], v[2], v[3], lit);
    if (pFacesInBatch) {
        (*pFacesInBatch)++;
#if defined(USE_RLGL)
        if (*pFacesInBatch >= 256) { rlDrawRenderBatchActive(); *pFacesInBatch = 0; }
#endif
    }
}

// Passe 2 — só linhas (wireframe). Shader padrão, fora do fog. pLinesInBatch: flush a cada 2048 linhas.
// Epsilon na normal: puxa linhas um tiquinho pra fora da face (evita z-fighting/diagonais no teto).
static void DrawBlockFace_Wire(float bx, float by, float bz, int faceDir, int* pLinesInBatch) {
    Vector3 v[4];
    if (!GetBlockFaceVertices(bx, by, bz, faceDir, v)) return;
    const float eps = 0.0015f;
    Vector3 n = GetFaceNormal(faceDir);
    for (int i = 0; i < 4; i++) v[i] = Vector3Add(v[i], Vector3Scale(n, eps));
    Color w = (Color){0, 0, 0, 80};
    DrawLine3D(v[0], v[1], w);
    DrawLine3D(v[1], v[2], w);
    DrawLine3D(v[2], v[3], w);
    DrawLine3D(v[3], v[0], w);
    DrawLine3D(v[0], v[2], w);
    if (pLinesInBatch) {
        *pLinesInBatch += 5;
#if defined(USE_RLGL)
        if (*pLinesInBatch >= 2048) { rlDrawRenderBatchActive(); *pLinesInBatch = 0; }
#endif
    }
}

// Estrutura de hitbox para blocos
typedef struct {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
} BlockAABB;

// Cria hitbox AABB para um bloco na posição (bx, by, bz)
static BlockAABB GetBlockAABB(float bx, float by, float bz) {
    BlockAABB aabb;
    aabb.minX = bx;
    aabb.maxX = bx + 1.0f;
    aabb.minY = by;
    aabb.maxY = by + 1.0f;
    aabb.minZ = bz;
    aabb.maxZ = bz + 1.0f;
    return aabb;
}

// Verifica colisão AABB entre player e bloco
static bool CheckAABBCollisionPlayerBlock(float px, float py, float pz, float pw, float ph, float pd,
                                          const BlockAABB* block) {
    float playerMinX = px - pw * 0.5f, playerMaxX = px + pw * 0.5f;
    float playerMinY = py, playerMaxY = py + ph;
    float playerMinZ = pz - pd * 0.5f, playerMaxZ = pz + pd * 0.5f;
    
    return !(playerMaxX < block->minX || playerMinX > block->maxX ||
             playerMaxY < block->minY || playerMinY > block->maxY ||
             playerMaxZ < block->minZ || playerMinZ > block->maxZ);
}

/* Colisão player vs nave por eixo (evita escalar parede / empurrão vertical falso). */
static void ResolvePlayerVsShipX(float* px, float py, float pz, float pw, float ph, float pd,
                                 const BoundingBox* shipBox) {
    (void)py; (void)pz; (void)ph; (void)pd;
    if (!px || !shipBox) return;
    float playerMinX = *px - pw * 0.5f, playerMaxX = *px + pw * 0.5f;
    float playerMinY = py, playerMaxY = py + ph;
    float playerMinZ = pz - pd * 0.5f, playerMaxZ = pz + pd * 0.5f;
    BoundingBox playerBox = { { playerMinX, playerMinY, playerMinZ }, { playerMaxX, playerMaxY, playerMaxZ } };
    if (!CheckCollisionBoxes(playerBox, *shipBox)) return;
    float pushLeft = playerMaxX - shipBox->min.x;
    float pushRight = shipBox->max.x - playerMinX;
    if (pushLeft < pushRight) *px -= pushLeft; else *px += pushRight;
}
static void ResolvePlayerVsShipZ(float px, float py, float* pz, float pw, float ph, float pd,
                                 const BoundingBox* shipBox) {
    (void)px; (void)py; (void)pw; (void)ph;
    if (!pz || !shipBox) return;
    float playerMinX = px - pw * 0.5f, playerMaxX = px + pw * 0.5f;
    float playerMinY = py, playerMaxY = py + ph;
    float playerMinZ = *pz - pd * 0.5f, playerMaxZ = *pz + pd * 0.5f;
    BoundingBox playerBox = { { playerMinX, playerMinY, playerMinZ }, { playerMaxX, playerMaxY, playerMaxZ } };
    if (!CheckCollisionBoxes(playerBox, *shipBox)) return;
    float pushBack = playerMaxZ - shipBox->min.z;
    float pushFwd = shipBox->max.z - playerMinZ;
    if (pushBack < pushFwd) *pz -= pushBack; else *pz += pushFwd;
}
static void ResolvePlayerVsShipY(float px, float* py, float pz, float pw, float ph, float pd,
                                 const BoundingBox* shipBox, bool* outStandingOnShip) {
    (void)px; (void)pz; (void)pw;
    if (!py || !shipBox || !outStandingOnShip) return;
    *outStandingOnShip = false;
    float playerMinX = px - pw * 0.5f, playerMaxX = px + pw * 0.5f;
    float playerMinY = *py, playerMaxY = *py + ph;
    float playerMinZ = pz - pd * 0.5f, playerMaxZ = pz + pd * 0.5f;
    BoundingBox playerBox = { { playerMinX, playerMinY, playerMinZ }, { playerMaxX, playerMaxY, playerMaxZ } };
    if (!CheckCollisionBoxes(playerBox, *shipBox)) return;
    float pushUp = shipBox->max.y - playerMinY;
    float pushDown = playerMaxY - shipBox->min.y;
    if (pushUp < pushDown) {
        *py += pushUp;
        *outStandingOnShip = true;
        g_playerPhysics.onGround = true;
    } else {
        *py -= pushDown;
    }
}

/* Resolve colisão AABB por eixo (só empurra no eixo dado; evita escalar parede). */
static void ResolveAABBCollisionX(float* px, float py, float pz, float pw, float ph, float pd,
                                  const BlockAABB* block) {
    (void)py; (void)pz; (void)ph; (void)pd;
    float playerMinX = *px - pw * 0.5f, playerMaxX = *px + pw * 0.5f;
    float playerMinY = py, playerMaxY = py + ph;
    float playerMinZ = pz - pd * 0.5f, playerMaxZ = pz + pd * 0.5f;
    if (playerMaxX < block->minX || playerMinX > block->maxX ||
        playerMaxY < block->minY || playerMinY > block->maxY ||
        playerMaxZ < block->minZ || playerMinZ > block->maxZ) return;
    float pushLeft = playerMaxX - block->minX;
    float pushRight = block->maxX - playerMinX;
    if (pushLeft < pushRight) *px -= pushLeft; else *px += pushRight;
}
static void ResolveAABBCollisionZ(float px, float py, float* pz, float pw, float ph, float pd,
                                  const BlockAABB* block) {
    (void)px; (void)py; (void)pw; (void)ph;
    float playerMinX = px - pw * 0.5f, playerMaxX = px + pw * 0.5f;
    float playerMinY = py, playerMaxY = py + ph;
    float playerMinZ = *pz - pd * 0.5f, playerMaxZ = *pz + pd * 0.5f;
    if (playerMaxX < block->minX || playerMinX > block->maxX ||
        playerMaxY < block->minY || playerMinY > block->maxY ||
        playerMaxZ < block->minZ || playerMinZ > block->maxZ) return;
    float pushBack = playerMaxZ - block->minZ;
    float pushFwd = block->maxZ - playerMinZ;
    if (pushBack < pushFwd) *pz -= pushBack; else *pz += pushFwd;
}
static void ResolveAABBCollisionY(float px, float* py, float pz, float pw, float ph, float pd,
                                  const BlockAABB* block) {
    (void)px; (void)pz; (void)pw;
    float playerMinX = px - pw * 0.5f, playerMaxX = px + pw * 0.5f;
    float playerMinY = *py, playerMaxY = *py + ph;
    float playerMinZ = pz - pd * 0.5f, playerMaxZ = pz + pd * 0.5f;
    if (playerMaxX < block->minX || playerMinX > block->maxX ||
        playerMaxY < block->minY || playerMinY > block->maxY ||
        playerMaxZ < block->minZ || playerMinZ > block->maxZ) return;
    float pushUp = block->maxY - playerMinY;
    float pushDown = playerMaxY - block->minY;
    if (pushUp < pushDown) {
        *py += pushUp;
        g_playerPhysics.onGround = true;
    } else {
        *py -= pushDown;
    }
}

/* Resolução por eixo (X, Z, Y): evita escalar parede e grudar no chão. Ordem: resolve X → Z → Y. */
static void ResolveCollisionsX(void) {
    g_playerPhysics.onGround = false;
    g_isColliding = false;

    float px = g_playerPhysics.x, py = g_playerPhysics.y, pz = g_playerPhysics.z;
    float pw = g_playerPhysics.width, ph = g_playerPhysics.height, pd = g_playerPhysics.depth;
    float playerMinX = px - pw * 0.5f, playerMaxX = px + pw * 0.5f;
    float playerMinY = py, playerMaxY = py + ph;
    float playerMinZ = pz - pd * 0.5f, playerMaxZ = pz + pd * 0.5f;

    int32_t minX = (int32_t)floorf(playerMinX) - 1, maxX = (int32_t)ceilf(playerMaxX) + 1;
    int32_t minY = (int32_t)floorf(playerMinY) - 1, maxY = (int32_t)ceilf(playerMaxY) + 1;
    int32_t minZ = (int32_t)floorf(playerMinZ) - 1, maxZ = (int32_t)ceilf(playerMaxZ) + 1;

    for (int32_t by = minY; by <= maxY; by++) {
        for (int32_t bz = minZ; bz <= maxZ; bz++) {
            for (int32_t bx = minX; bx <= maxX; bx++) {
                if (!IsBlockSolid(bx, by, bz)) continue;
                BlockAABB blockAABB = GetBlockAABB((float)bx, (float)by, (float)bz);
                if (CheckAABBCollisionPlayerBlock(px, py, pz, pw, ph, pd, &blockAABB)) {
                    g_isColliding = true;
                    ResolveAABBCollisionX(&g_playerPhysics.x, py, pz, pw, ph, pd, &blockAABB);
                    px = g_playerPhysics.x;
                }
            }
        }
    }

    /* Casco: top-only — resolve lateral (X) só quando player está abaixo do topo do hull. */
    if (g_useStreamingWorld && g_voxelWorld) {
        float hullTop = g_ship.hullBox.max.y;
        float playerBottom = py;  /* pés do player. */
        if (playerBottom < hullTop - COLLISION_ABOVE_HULL_EPS) {
            if (CheckCollisionBoxes((BoundingBox){ { g_playerPhysics.x - pw*0.5f, playerMinY, playerMinZ }, { g_playerPhysics.x + pw*0.5f, playerMaxY, playerMaxZ } }, g_ship.hullBox)) {
                g_isColliding = true;
                ResolvePlayerVsShipX(&g_playerPhysics.x, py, pz, pw, ph, pd, &g_ship.hullBox);
            }
        }
    }
}

static void ResolveCollisionsZ(void) {
    float px = g_playerPhysics.x, py = g_playerPhysics.y, pz = g_playerPhysics.z;
    float pw = g_playerPhysics.width, ph = g_playerPhysics.height, pd = g_playerPhysics.depth;
    float playerMinX = px - pw * 0.5f, playerMaxX = px + pw * 0.5f;
    float playerMinY = py, playerMaxY = py + ph;
    float playerMinZ = pz - pd * 0.5f, playerMaxZ = pz + pd * 0.5f;

    int32_t minX = (int32_t)floorf(playerMinX) - 1, maxX = (int32_t)ceilf(playerMaxX) + 1;
    int32_t minY = (int32_t)floorf(playerMinY) - 1, maxY = (int32_t)ceilf(playerMaxY) + 1;
    int32_t minZ = (int32_t)floorf(playerMinZ) - 1, maxZ = (int32_t)ceilf(playerMaxZ) + 1;

    for (int32_t by = minY; by <= maxY; by++) {
        for (int32_t bz = minZ; bz <= maxZ; bz++) {
            for (int32_t bx = minX; bx <= maxX; bx++) {
                if (!IsBlockSolid(bx, by, bz)) continue;
                BlockAABB blockAABB = GetBlockAABB((float)bx, (float)by, (float)bz);
                if (CheckAABBCollisionPlayerBlock(px, py, pz, pw, ph, pd, &blockAABB)) {
                    g_isColliding = true;
                    ResolveAABBCollisionZ(px, py, &g_playerPhysics.z, pw, ph, pd, &blockAABB);
                    pz = g_playerPhysics.z;
                }
            }
        }
    }

    /* Casco: top-only — resolve lateral (Z) só quando player está abaixo do topo do hull. */
    if (g_useStreamingWorld && g_voxelWorld) {
        float hullTop = g_ship.hullBox.max.y;
        float playerBottom = py;
        if (playerBottom < hullTop - COLLISION_ABOVE_HULL_EPS) {
            if (CheckCollisionBoxes((BoundingBox){ { playerMinX, playerMinY, g_playerPhysics.z - pd*0.5f }, { playerMaxX, playerMaxY, g_playerPhysics.z + pd*0.5f } }, g_ship.hullBox)) {
                g_isColliding = true;
                ResolvePlayerVsShipZ(px, py, &g_playerPhysics.z, pw, ph, pd, &g_ship.hullBox);
            }
        }
    }
}

static void ResolveCollisionsY(void) {
    g_standingOnShip = false;
    g_isOnLadder = false;

    float px = g_playerPhysics.x, py = g_playerPhysics.y, pz = g_playerPhysics.z;
    float pw = g_playerPhysics.width, ph = g_playerPhysics.height, pd = g_playerPhysics.depth;
    float playerMinX = px - pw * 0.5f, playerMaxX = px + pw * 0.5f;
    float playerMinY = py, playerMaxY = py + ph;
    float playerMinZ = pz - pd * 0.5f, playerMaxZ = pz + pd * 0.5f;

    int32_t minX = (int32_t)floorf(playerMinX) - 1, maxX = (int32_t)ceilf(playerMaxX) + 1;
    int32_t minY = (int32_t)floorf(playerMinY) - 1, maxY = (int32_t)ceilf(playerMaxY) + 1;
    int32_t minZ = (int32_t)floorf(playerMinZ) - 1, maxZ = (int32_t)ceilf(playerMaxZ) + 1;

    for (int32_t by = minY; by <= maxY; by++) {
        for (int32_t bz = minZ; bz <= maxZ; bz++) {
            for (int32_t bx = minX; bx <= maxX; bx++) {
                if (!IsBlockSolid(bx, by, bz)) continue;
                BlockAABB blockAABB = GetBlockAABB((float)bx, (float)by, (float)bz);
                if (CheckAABBCollisionPlayerBlock(px, py, pz, pw, ph, pd, &blockAABB)) {
                    g_isColliding = true;
                    ResolveAABBCollisionY(px, &g_playerPhysics.y, pz, pw, ph, pd, &blockAABB);
                    py = g_playerPhysics.y;
                }
            }
        }
    }

    if (g_useStreamingWorld && g_voxelWorld) {
        float pyNow = g_playerPhysics.y;
        BoundingBox playerBox = {
            { px - pw*0.5f, pyNow, pz - pd*0.5f },
            { px + pw*0.5f, pyNow + ph, pz + pd*0.5f }
        };
        if (CheckCollisionBoxes(playerBox, g_ship.hullBox)) {
            g_isColliding = true;
            ResolvePlayerVsShipY(px, &g_playerPhysics.y, pz, pw, ph, pd, &g_ship.hullBox, &g_standingOnShip);
        }
        playerBox.min.x = g_playerPhysics.x - pw * 0.5f;
        playerBox.max.x = g_playerPhysics.x + pw * 0.5f;
        playerBox.min.y = g_playerPhysics.y;
        playerBox.max.y = g_playerPhysics.y + ph;
        playerBox.min.z = g_playerPhysics.z - pd * 0.5f;
        playerBox.max.z = g_playerPhysics.z + pd * 0.5f;
        if (CheckCollisionBoxes(playerBox, g_ship.deckBox)) {
            g_standingOnShip = true;
            g_playerPhysics.onGround = true;
        }
        if (CheckCollisionBoxes(playerBox, g_ship.ladderBox) && g_playerPhysics.vy >= 0.0f) {
            g_isOnLadder = true;
        }
        if (CheckCollisionBoxes(playerBox, g_ship.deckBox)) {
            g_isOnLadder = false;
            g_standingOnShip = true;
            g_playerPhysics.vy = 0.0f;
        }
        /* Standing só conta se pé perto do topo do hull e XZ dentro do hull (evita standing=1 meio fora). */
        if (g_standingOnShip) {
            float hullTop = g_ship.hullBox.max.y;
            float footY = g_playerPhysics.y;
            int inHullXZ = (g_playerPhysics.x >= g_ship.hullBox.min.x && g_playerPhysics.x <= g_ship.hullBox.max.x &&
                            g_playerPhysics.z >= g_ship.hullBox.min.z && g_playerPhysics.z <= g_ship.hullBox.max.z);
            if (fabsf(footY - hullTop) > COLLISION_ABOVE_HULL_EPS || !inHullXZ)
                g_standingOnShip = false;
        }
    }

    if (g_playerPhysics.onGround) g_isColliding = true;
}

/* ——— Bússola (direção do olhar no plano XZ). Mundo: +Z=Norte, -Z=Sul, +X=Leste, -X=Oeste. ——— */
#ifndef PI
#define PI 3.14159265358979323846f
#endif
static const char* GetCardinal(float deg) {
    if (deg >= 315.0f || deg < 45.0f)  return "NORTE";
    if (deg < 135.0f)                   return "LESTE";
    if (deg < 225.0f)                   return "SUL";
    return "OESTE";
}
static void Debug_DrawCompass(const FPSCamera* cam) {
    if (!cam) return;
    float forwardX, forwardZ;
    FPSCamera_GetForwardFlat(cam, &forwardX, &forwardZ);
    float lenSq = forwardX * forwardX + forwardZ * forwardZ;
    if (lenSq < 0.000001f) return;
    float angleRad = atan2f(forwardX, forwardZ);
    float angleDeg = angleRad * (180.0f / PI);
    if (angleDeg < 0.0f) angleDeg += 360.0f;
    const char* dir = GetCardinal(angleDeg);
    DrawText(TextFormat("DIR: %s (%.1f°)", dir, angleDeg), 20, 60, 20, YELLOW);
}

// Gera um mapa simples de debug - SEM chunks, SEM sistema procedural
static void GenerateDebugMap(void) {
    // Limpa todo o mapa (tudo vira ar)
    for (int32_t x = 0; x < MAP_SIZE_X; x++) {
        for (int32_t y = 0; y < MAP_SIZE_Y; y++) {
            for (int32_t z = 0; z < MAP_SIZE_Z; z++) {
                g_map[x][y][z] = BLOCK_AIR;
            }
        }
    }
    
    const int32_t floorY = 0; // Chão na coordenada Y=0
    const int32_t mapSize = 20; // Mapa 20x20 blocos
    
    // 1. Chão plano (Y=0)
    for (int32_t x = -mapSize; x <= mapSize; x++) {
        for (int32_t z = -mapSize; z <= mapSize; z++) {
            int32_t mapX = x + MAP_OFFSET_X;
            int32_t mapZ = z + MAP_OFFSET_Z;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z) {
                g_map[mapX][floorY][mapZ] = BLOCK_TERRAIN;
            }
        }
    }
    
    // 2. Paredes ao redor (bordas) - altura 5 blocos
    for (int32_t y = floorY; y <= floorY + 5; y++) {
        // Parede norte (z = -mapSize)
        for (int32_t x = -mapSize; x <= mapSize; x++) {
            int32_t mapX = x + MAP_OFFSET_X;
            int32_t mapZ = (-mapSize) + MAP_OFFSET_Z;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
        // Parede sul (z = mapSize)
        for (int32_t x = -mapSize; x <= mapSize; x++) {
            int32_t mapX = x + MAP_OFFSET_X;
            int32_t mapZ = mapSize + MAP_OFFSET_Z;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
        // Parede oeste (x = -mapSize)
        for (int32_t z = -mapSize; z <= mapSize; z++) {
            int32_t mapX = (-mapSize) + MAP_OFFSET_X;
            int32_t mapZ = z + MAP_OFFSET_Z;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
        // Parede leste (x = mapSize)
        for (int32_t z = -mapSize; z <= mapSize; z++) {
            int32_t mapX = mapSize + MAP_OFFSET_X;
            int32_t mapZ = z + MAP_OFFSET_Z;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
    }
    
    // 3. Estrutura central com escadas
    // Plataforma elevada no centro (Y=3)
    for (int32_t x = -5; x <= 5; x++) {
        for (int32_t z = -5; z <= 5; z++) {
            int32_t mapX = x + MAP_OFFSET_X;
            int32_t mapZ = z + MAP_OFFSET_Z;
            int32_t y = floorY + 3;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
    }
    
    // Escada subindo para a plataforma (lado norte)
    for (int32_t step = 0; step < 3; step++) {
        for (int32_t x = -2; x <= 2; x++) {
            int32_t mapX = x + MAP_OFFSET_X;
            int32_t mapZ = (-5 - step) + MAP_OFFSET_Z;
            int32_t y = floorY + step;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
    }
    
    // Escada descendo da plataforma (lado sul)
    for (int32_t step = 0; step < 3; step++) {
        for (int32_t x = -2; x <= 2; x++) {
            int32_t mapX = x + MAP_OFFSET_X;
            int32_t mapZ = (5 + step) + MAP_OFFSET_Z;
            int32_t y = floorY + 3 - step;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
    }
    
    // 4. Algumas colunas/pilares para teste
    for (int32_t i = 0; i < 4; i++) {
        int32_t px = -15 + (i % 2) * 30;
        int32_t pz = -15 + (i / 2) * 30;
        for (int32_t y = floorY; y <= floorY + 8; y++) {
            int32_t mapX = px + MAP_OFFSET_X;
            int32_t mapZ = pz + MAP_OFFSET_Z;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
    }
    
    // 5. Uma parede divisória
    for (int32_t y = floorY; y <= floorY + 4; y++) {
        for (int32_t z = -10; z <= 10; z++) {
            int32_t mapX = 0 + MAP_OFFSET_X;
            int32_t mapZ = z + MAP_OFFSET_Z;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_GRAY;
            }
        }
    }
    // Abertura na parede (deixa espaço para passar)
    for (int32_t y = floorY; y <= floorY + 2; y++) {
        for (int32_t z = -2; z <= 2; z++) {
            int32_t mapX = 0 + MAP_OFFSET_X;
            int32_t mapZ = z + MAP_OFFSET_Z;
            if (mapX >= 0 && mapX < MAP_SIZE_X && mapZ >= 0 && mapZ < MAP_SIZE_Z && y < MAP_SIZE_Y) {
                g_map[mapX][y][mapZ] = BLOCK_AIR;
            }
        }
    }
    
    // 6. Bloco vermelho no spawn (origem)
    int32_t spawnX = 0 + MAP_OFFSET_X;
    int32_t spawnZ = 0 + MAP_OFFSET_Z;
    if (spawnX >= 0 && spawnX < MAP_SIZE_X && spawnZ >= 0 && spawnZ < MAP_SIZE_Z) {
        g_map[spawnX][floorY][spawnZ] = BLOCK_RED;
    }
}

void Scene_Gameplay_Shutdown(void) {
    if (!g_initialized) return;
    g_mode = GP_PLAYING;
    g_pausePage = PAUSE_MAIN;
    g_pauseSelectionMain = 0;
    g_pauseSelectionSettings = 0;
    g_pauseSelectionBinds = 0;
    g_waitingForKey = false;
    g_uiAnimInit = false;
    g_draggingSlider = -1;
    if (g_fpsCamera.locked) {
        FPSCamera_UnlockMouse(&g_fpsCamera);
    }
    
    // Reseta física do player
    memset(&g_playerPhysics, 0, sizeof(PhysicsBody));
    
    // Limpa mapa
    for (int32_t x = 0; x < MAP_SIZE_X; x++) {
        for (int32_t y = 0; y < MAP_SIZE_Y; y++) {
            for (int32_t z = 0; z < MAP_SIZE_Z; z++) {
                g_map[x][y][z] = BLOCK_AIR;
            }
        }
    }
    
    // Limpa sistemas de renderização
    Atmosphere_Shutdown(&g_atmosphere);
    Lighting_Shutdown(&g_lighting);
    if (g_fogShader.id != 0) {
        UnloadShader(g_fogShader);
        g_fogShader.id = 0;
        g_fogLocStart = g_fogLocEnd = g_fogLocColor = g_fogLocType = g_fogLocDensity = -1;
        g_fogLocHorizon = g_fogLocSky = g_fogLocH0 = g_fogLocHRange = -1;
    }
    if (g_skyShader.id != 0) {
        UnloadShader(g_skyShader);
        g_skyShader.id = 0;
        g_skyLocBottom = g_skyLocHorizon = g_skyLocTop = -1;
    }
    if (g_shipModelLoaded) {
        UnloadModel(g_shipModel);
        g_shipModelLoaded = false;
    }
    if (g_crtTarget.id != 0) {
        UnloadRenderTexture(g_crtTarget);
        g_crtTarget = (RenderTexture2D){0};
    }
    if (g_crtShader.id != 0) {
        UnloadShader(g_crtShader);
        g_crtShader = (Shader){0};
    }
    if (g_terminalCrtShader.id != 0) {
        UnloadShader(g_terminalCrtShader);
        g_terminalCrtShader = (Shader){0};
        g_terminalCrtTimeLoc = -1;
    }
    if (g_voxelWorld) {
        VoxelWorld_Destroy(g_voxelWorld);
        g_voxelWorld = NULL;
    }
    if (g_arcTerminalFull) {
        ArcTerminalFull_Destroy(g_arcTerminalFull);
        g_arcTerminalFull = NULL;
    }

    // Reseta flags
    g_initialized = false;
    g_firstFrame = true;
}

void Scene_Gameplay_Init(void) {
    // Se já estava inicializado, limpa primeiro
    if (g_initialized) {
        Scene_Gameplay_Shutdown();
    }
    
    if (!g_useStreamingWorld) {
        GenerateDebugMap();
    } else {
        g_voxelWorld = VoxelWorld_Create("beware-the-dust");
        WorldBeware_Init(&g_worldBeware, "beware-the-dust");
        WorldBeware_AttachVoxelWorld(&g_worldBeware, g_voxelWorld);
        Ship_Init(&g_ship);
    }
    
    /* Spawn: centro do mapa ou em cima do deck da nave (streaming). */
    const float spawnX = 0.0f;
    const float spawnY = 1.0f;
    const float spawnZ = 0.0f;
    PhysicsBody_Init(&g_playerPhysics, spawnX, spawnY, spawnZ);
    if (g_useStreamingWorld && g_voxelWorld) {
        /* Jogador spawna em cima da nave, na mesma posição que o pouso começa. */
        g_playerPhysics.x = g_ship.position.x;
        g_playerPhysics.z = g_ship.position.z;
        g_playerPhysics.y = g_ship.hullBox.max.y + COLLISION_ABOVE_HULL_EPS;
        g_playerPhysics.vx = g_playerPhysics.vy = g_playerPhysics.vz = 0.0f;
    }
    g_playerPhysics.onGround = true;
    g_playerPhysics.vy = 0.0f;
    
    // Carrega fonte de assets/fonts (menu in-game e HUD)
    const char* fontPath = GetAssetPath("assets/fonts/CONSOLA.TTF");
    g_consolaFont = LoadFont(fontPath);
    if (g_consolaFont.texture.id == 0) {
        const char* alt = GetAssetPath("assets/fonts/CONSOLA.ttf");
        g_consolaFont = LoadFont(alt);
    }
    if (g_consolaFont.texture.id != 0) {
        SetTextureFilter(g_consolaFont.texture, TEXTURE_FILTER_BILINEAR);
    }
    
    if (g_consolaFont.texture.id == 0) {
        TraceLog(LOG_WARNING, "Falha ao carregar fonte CONSOLA de assets/fonts! Usando fonte padrão.");
        g_consolaFont = GetFontDefault();
    }
    
    // CRT overlay: RT + shader (só desenha por cima da tela, não altera gameplay)
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    if (sw < 1) sw = 1;
    if (sh < 1) sh = 1;
    g_crtTarget = LoadRenderTexture(sw, sh);
    {
        const char* crtPath = GetAssetPath("assets/shaders/menu-crt.fs");
        g_crtShader = LoadShader(0, crtPath);
        if (g_crtShader.id == 0) {
            TraceLog(LOG_WARNING, "Gameplay: CRT overlay shader nao carregado (tentou %s).", crtPath);
        }
        /* CRT do terminal ARC (crt.fs do terminal-with-raylib). */
        const char* terminalCrtPath = GetAssetPath("assets/shaders/crt.fs");
        g_terminalCrtShader = LoadShader(0, terminalCrtPath);
        if (g_terminalCrtShader.id != 0) {
            g_terminalCrtTimeLoc = GetShaderLocation(g_terminalCrtShader, "time");
        }
    }
    
    // Inicializa câmera FPS estilo Minecraft
    FPSCamera_Init(&g_fpsCamera, g_playerPhysics.x, g_playerPhysics.y + PLAYER_EYE_HEIGHT, g_playerPhysics.z);
    
    // Inicializa sistema de atmosfera (fog + sky gradient)
    Atmosphere_Init(&g_atmosphere);
    // Fog exponencial no forward (sci-fi): fogT = 1 - exp(-density*d)
    Atmosphere_SetFog(&g_atmosphere, true, FOG_EXPONENTIAL, 20.0f, 50.0f, 0.02f,
                       (Color){20, 20, 30, 255});
    // Configura sky gradient (mantém o que já estava bom)
    Atmosphere_SetSkyGradient(&g_atmosphere,
                               (Color){30, 30, 50, 255},      // Topo: azul escuro
                               (Color){20, 20, 30, 255},      // Horizonte: azul muito escuro
                               (Color){15, 15, 25, 255});     // Fundo: quase preto
    
    // Nave 3D para o lobby (só a nave renderiza, jogador anda dentro): assets/models/ship.glb
    const char* shipPath = GetAssetPath("assets/models/ship.glb");
    if (FileExists(shipPath)) {
        g_shipModel = LoadModel(shipPath);
        g_shipModelLoaded = true;
        /* GLTF sem textura → material branco padrão; forçar cinza para não estourar */
        for (int i = 0; i < g_shipModel.materialCount; i++) {
            g_shipModel.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = (Color){ 180, 180, 180, 255 };
        }
        // Se tiver textura depois: g_shipModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("assets/textures/ship_diffuse.png");
        TraceLog(LOG_INFO, "Ship model loaded: %s", shipPath);
    } else {
        TraceLog(LOG_WARNING, "Ship model not found: %s (lobby will show no ship)", shipPath);
    }

    /* Terminal ARC: overlay completo boot/auth/loading/shell ao pressionar E. */
    if (g_useStreamingWorld) {
        g_arcTerminalFull = ArcTerminalFull_Create();
        if (g_arcTerminalFull) TraceLog(LOG_INFO, "ARC terminal full initialized.");
    }

    // Inicializa sistema de iluminação direcional fake
    Lighting_Init(&g_lighting);
    // Configura luz direcional (sol vindo do alto e ligeiramente de frente)
    Lighting_SetDirectionalLight(&g_lighting,
                                 (Vector3){0.3f, -0.8f, 0.5f}, // Direção normalizada
                                 (Color){255, 250, 240, 255},  // Cor da luz (branco quente)
                                 (Color){50, 50, 60, 255},     // Cor ambiente (azul escuro)
                                 0.8f,                          // Intensidade (80%)
                                 0.3f);                         // Intensidade ambiente (30%)

    // Fog no forward (view-space): shader com d = -viewPos.z, sem depth texture
    const char* vsPath = GetAssetPath("assets/shaders/fog_forward.vs");
    const char* fsPath = GetAssetPath("assets/shaders/fog_forward.fs");
    if (FileExists(vsPath) && FileExists(fsPath)) {
        g_fogShader = LoadShader(vsPath, fsPath);
        if (g_fogShader.id != 0) {
            g_fogLocStart = GetShaderLocation(g_fogShader, "fogStart");
            g_fogLocEnd = GetShaderLocation(g_fogShader, "fogEnd");
            g_fogLocColor = GetShaderLocation(g_fogShader, "fogColor");
            g_fogLocType = GetShaderLocation(g_fogShader, "fogType");
            g_fogLocDensity = GetShaderLocation(g_fogShader, "fogDensity");
            g_fogLocHorizon = GetShaderLocation(g_fogShader, "fogHorizonColor");
            g_fogLocSky = GetShaderLocation(g_fogShader, "fogSkyColor");
            g_fogLocH0 = GetShaderLocation(g_fogShader, "fogH0");
            g_fogLocHRange = GetShaderLocation(g_fogShader, "fogHRange");
        }
    }
    if (g_fogShader.id == 0) {
        TraceLog(LOG_WARNING, "Fog forward shader nao carregado; fog em CPU desativado.");
    }

    // Sky gradient por direção do raio (esfera, sem depth texture)
    vsPath = GetAssetPath("assets/shaders/sky_gradient.vs");
    fsPath = GetAssetPath("assets/shaders/sky_gradient.fs");
    if (FileExists(vsPath) && FileExists(fsPath)) {
        g_skyShader = LoadShader(vsPath, fsPath);
        if (g_skyShader.id != 0) {
            g_skyLocBottom = GetShaderLocation(g_skyShader, "skyBottom");
            g_skyLocHorizon = GetShaderLocation(g_skyShader, "skyHorizon");
            g_skyLocTop = GetShaderLocation(g_skyShader, "skyTop");
        }
    }
    if (g_skyShader.id == 0) {
        TraceLog(LOG_WARNING, "Sky gradient shader nao carregado; usando Atmosphere_DrawSky.");
    }
    
    // Inicializa modo no clip como desativado
    g_noClip = false;
    
    g_firstFrame = true;
    DisableCursor();
    g_initialized = true;
}

void Scene_Gameplay_Update(float dt) {
    if (!g_initialized) return;

    if (IsKeyPressed(KEY_F1)) g_dbgShowGrid = !g_dbgShowGrid;
    if (IsKeyPressed(KEY_F2)) g_showDirection = !g_showDirection;
    if (IsKeyPressed(KEY_F3)) g_dbgShowStats = !g_dbgShowStats;
    if (IsKeyPressed(KEY_F4)) g_dbgShowWireframe = !g_dbgShowWireframe;

    // Trava mouse na primeira frame da gameplay
    if (g_firstFrame) {
        FPSCamera_LockMouse(&g_fpsCamera);
        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;
        SetMousePosition(centerX, centerY);
        // Limpa o delta do mouse após centralizar
        GetMouseDelta(); // Chama uma vez para resetar o delta
        g_firstFrame = false;
    }
    
    if (g_mode == GP_PAUSED) {
        if (g_waitingForKey) {
            int k = GetKeyPressed();
            if (k != 0) {
                if (k == KEY_ESCAPE) {
                    g_waitingForKey = false;
                } else {
                    g_settings.keys[g_bindingAction].primary = k;
                    g_waitingForKey = false;
                }
            }
        } else {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE)) {
                if (g_pausePage == PAUSE_KEYBINDS) g_pausePage = PAUSE_SETTINGS;
                else if (g_pausePage == PAUSE_SETTINGS) g_pausePage = PAUSE_MAIN;
                else if (g_pausePage == PAUSE_MAIN) {
                    g_mode = GP_PLAYING;
                    DisableCursor();
                    FPSCamera_LockMouse(&g_fpsCamera);
                    SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
                }
            } else {
                /* Coordenadas do mouse no espaço do RT (menu é desenhado no RT; alinha cursor com áreas clicáveis) */
                int rtw = g_crtTarget.texture.width;
                int rth = g_crtTarget.texture.height;
                int winW = GetScreenWidth();
                int winH = GetScreenHeight();
                if (winW < 1) winW = 1;
                if (winH < 1) winH = 1;
                Vector2 mouse;
                mouse.x = (float)GetMouseX() * (float)rtw / (float)winW;
                mouse.y = (float)GetMouseY() * (float)rth / (float)winH;
                float cx = (float)rtw / 2.0f;
                float cy = (float)rth / 2.0f;
                float scroll = GetMouseWheelMove();

                if (g_pausePage == PAUSE_MAIN) {
                    float startY = cy - 36.0f;
                    float rowH = 40.0f;
                    float panelW = 280.0f;
                    for (int i = 0; i < 3; i++) {
                        Rectangle r = { cx - panelW/2, startY + (float)i * rowH - 4, panelW, rowH + 8 };
                        if (CheckCollisionPointRec(mouse, r)) { g_pauseSelectionMain = i; break; }
                    }
                    if (scroll > 0) g_pauseSelectionMain = (g_pauseSelectionMain + 2) % 3;
                    if (scroll < 0) g_pauseSelectionMain = (g_pauseSelectionMain + 1) % 3;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                        if (g_pauseSelectionMain == 0) {
                            g_mode = GP_PLAYING;
                            DisableCursor();
                            FPSCamera_LockMouse(&g_fpsCamera);
                            SetMousePosition(GetScreenWidth() / 2, GetScreenHeight() / 2);
                        } else if (g_pauseSelectionMain == 1) { g_pausePage = PAUSE_SETTINGS; g_pauseSelectionSettings = 0; }
                        else { SceneManager_Change(SCENE_MENU_MAIN); return; }
                    }
                } else if (g_pausePage == PAUSE_SETTINGS) {
                    if (!g_uiAnimInit) {
                        g_uiAnim.fovVis = g_settings.fov;
                        g_uiAnim.sensVis = g_settings.mouseSensitivity;
                        g_uiAnimInit = true;
                    }
                    { float dt = GetFrameTime(); g_uiAnim.fovVis = SmoothTo(g_uiAnim.fovVis, g_settings.fov, 18.0f, dt); g_uiAnim.sensVis = SmoothTo(g_uiAnim.sensVis, g_settings.mouseSensitivity, 18.0f, dt); }
                    float startY = cy - 100.0f;
                    const float rowH = 44.0f;
                    const float panelW = 420.0f;
                    const float sliderW = 200.0f;
                    const float sliderX = cx - 100.0f;
                    Rectangle rectFovSlider = { sliderX, startY + 24.0f, sliderW, 16.0f };
                    Rectangle rectSensSlider = { sliderX, startY + 24.0f + rowH, sliderW, 16.0f };

                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) g_draggingSlider = -1;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (CheckCollisionPointRec(mouse, rectFovSlider)) g_draggingSlider = SET_FOV;
                        else if (CheckCollisionPointRec(mouse, rectSensSlider)) g_draggingSlider = SET_SENS;
                    }
                    if (g_draggingSlider >= 0 && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                        float mx = mouse.x;
                        float t = (mx - sliderX) / sliderW;
                        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;
                        if (g_draggingSlider == SET_FOV) {
                            g_settings.fov = 60.0f + t * 50.0f;
                        } else if (g_draggingSlider == SET_SENS) {
                            g_settings.mouseSensitivity = 0.05f + t * 0.95f;
                        }
                        Settings_Clamp();
                    } else {
                        for (int i = 0; i < SET_COUNT; i++) {
                            Rectangle r = { cx - panelW/2, startY + (float)i * rowH - 6, panelW, rowH + 12 };
                            if (CheckCollisionPointRec(mouse, r)) { g_pauseSelectionSettings = i; break; }
                        }
                        if (scroll > 0) g_pauseSelectionSettings = (g_pauseSelectionSettings + SET_COUNT - 1) % SET_COUNT;
                        if (scroll < 0) g_pauseSelectionSettings = (g_pauseSelectionSettings + 1) % SET_COUNT;
                        if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                            if (g_pauseSelectionSettings == SET_FOV) { g_settings.fov -= 2.0f; }
                            if (g_pauseSelectionSettings == SET_SENS) { g_settings.mouseSensitivity -= 0.05f; }
                            Settings_Clamp();
                        }
                        if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                            if (g_pauseSelectionSettings == SET_FOV) { g_settings.fov += 2.0f; }
                            if (g_pauseSelectionSettings == SET_SENS) { g_settings.mouseSensitivity += 0.05f; }
                            Settings_Clamp();
                        }
                        if ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && g_draggingSlider < 0) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                            if (g_pauseSelectionSettings == SET_CROSSHAIR) g_settings.crosshair = (g_settings.crosshair == CROSSHAIR_DOT) ? CROSSHAIR_PLUS : CROSSHAIR_DOT;
                            if (g_pauseSelectionSettings == SET_CONTROLS) { g_pausePage = PAUSE_KEYBINDS; g_pauseSelectionBinds = 0; }
                            if (g_pauseSelectionSettings == SET_BACK) g_pausePage = PAUSE_MAIN;
                        }
                    }
                } else if (g_pausePage == PAUSE_KEYBINDS) {
                    const int BINDS_COUNT = ACT_COUNT + 1;
                    float startY = cy - 80.0f;
                    float rowH = 26.0f;
                    for (int i = 0; i < BINDS_COUNT; i++) {
                        Rectangle r = { cx - 200, startY + (float)i * rowH - 2, 400, rowH + 4 };
                        if (CheckCollisionPointRec(mouse, r)) { g_pauseSelectionBinds = i; break; }
                    }
                    if (scroll > 0) g_pauseSelectionBinds = (g_pauseSelectionBinds + BINDS_COUNT - 1) % BINDS_COUNT;
                    if (scroll < 0) g_pauseSelectionBinds = (g_pauseSelectionBinds + 1) % BINDS_COUNT;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                        if (g_pauseSelectionBinds < ACT_COUNT) {
                            g_bindingAction = (ActionId)g_pauseSelectionBinds;
                            g_waitingForKey = true;
                        } else {
                            g_pausePage = PAUSE_SETTINGS;
                        }
                    }
                }
            }
        }
        return;
    }

    if (KeyPressed(ACT_PAUSE)) {
        g_mode = GP_PAUSED;
        EnableCursor();
        FPSCamera_UnlockMouse(&g_fpsCamera);
        g_pausePage = PAUSE_MAIN;
        return;
    }

    if (g_mode != GP_PLAYING) return;

    /* Terminal ARC: E perto do monitor = toggle overlay. */
    if (g_arcTerminalFull && g_useStreamingWorld && g_voxelWorld) {
        float mx = g_ship.position.x, mz = g_ship.position.z + 1.5f;
        float dx = g_playerPhysics.x - mx, dz = g_playerPhysics.z - mz;
        float distSq = dx * dx + dz * dz;
        bool nearMonitor = (distSq < MONITOR_INTERACT_DIST * MONITOR_INTERACT_DIST);

        if (ArcTerminalFull_IsOpen(g_arcTerminalFull)) {
            EnableCursor();
            FPSCamera_UnlockMouse(&g_fpsCamera);
            ArcTerminalFull_Update(g_arcTerminalFull, dt);
            /* GetCharPressed: caracteres (evita duplicação; não processar teclas via GetKeyPressed). */
            int k = GetCharPressed();
            while (k > 0) {
                ArcTerminalFull_ProcessKey(g_arcTerminalFull, k);
                k = GetCharPressed();
            }
            /* GetKeyPressed: apenas teclas especiais (ENTER, BACKSPACE, setas, TAB, etc.); ignorar letras/números. */
            k = GetKeyPressed();
            while (k != 0) {
                if (k < 32 || k > 126)
                    ArcTerminalFull_ProcessKey(g_arcTerminalFull, k);
                k = GetKeyPressed();
            }
            if (IsKeyPressed(KEY_E) && ArcTerminalFull_IsInShell(g_arcTerminalFull)) {
                ArcTerminalFull_Close(g_arcTerminalFull);
                DisableCursor();
                FPSCamera_LockMouse(&g_fpsCamera);
            }
            g_fpsCamera.fov = g_settings.fov;
            g_fpsCamera.position.x = g_playerPhysics.x;
            g_fpsCamera.position.y = g_playerPhysics.y + PLAYER_EYE_HEIGHT;
            g_fpsCamera.position.z = g_playerPhysics.z;
            return;  /* Não processa movimento quando terminal aberto. */
        } else if (nearMonitor && IsKeyPressed(KEY_E)) {
            ArcTerminalFull_Open(g_arcTerminalFull);
        }
        g_nearArcMonitor = nearMonitor;
    } else {
        g_nearArcMonitor = false;
    }

    g_fpsCamera.fov = g_settings.fov;
    g_fpsCamera.sensitivity = g_settings.mouseSensitivity;

    if (g_fpsCamera.locked) {
        Vector2 mouseDelta = GetMouseDelta();
        FPSCamera_UpdateMouse(&g_fpsCamera, mouseDelta.x, mouseDelta.y);
        /* Sway lateral ao mover o mouse (suave e pouco perceptível; decai rápido) */
        g_fpsCamera.roll += mouseDelta.x * 0.022f;
        if (g_fpsCamera.roll > 1.8f) g_fpsCamera.roll = 1.8f;
        if (g_fpsCamera.roll < -1.8f) g_fpsCamera.roll = -1.8f;
        g_fpsCamera.roll *= 0.93f;
        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;
        SetMousePosition(centerX, centerY);
    }
    
    if (IsKeyPressed(KEY_N)) {
        g_noClip = !g_noClip;
    }
    
    /* Nave atualiza primeiro (clamp ao chão está dentro de Ship_Update). */
    if (g_useStreamingWorld && g_voxelWorld) {
        Ship_Update(&g_ship, dt);
        Ship_UpdateCollision(&g_ship);
        /* Delta aplicado aqui (antes do movimento) usando standingOnShip do frame anterior.
         * Herdar deltaY evita kick/jitter quando a nave desce (player acompanha em Y).
         * Quando speed > 0: se o player estiver na borda, o delta pode tirá-lo da projeção XZ,
         * standing vira false e ele cai — comportamento correto de plataforma móvel, não bug. */
        if (g_standingOnShip) {
            g_playerPhysics.x += g_ship.deltaX;
            g_playerPhysics.y += g_ship.deltaY;
            g_playerPhysics.z += g_ship.deltaZ;
            if (g_ship.deltaY != 0.0f) g_playerPhysics.vy = 0.0f;
        }
    }
    
    /* F5: inicia Drop Sequence (descida curva); player livre antes/durante/depois. */
    if (g_useStreamingWorld && g_voxelWorld && IsKeyPressed(KEY_F5) && g_ship.state == SHIP_IDLE_HOVER) {
        g_ship.state = SHIP_DESCENDING;
        g_ship.descendTimer = 0.0f;
        /* Destino: pairar perto do chão no centro do corredor (landZ fixo = 0). */
        float groundY = 0.0f;
        float finalY = groundY + g_ship.targetHeight;
        float landX = 0.0f;
        float landZ = 0.0f;
        g_ship.descendEndPos = (Vector3){ landX, finalY, landZ };
        /* Início do arco: alto e atrás (-30 em Z); nave já está aqui ao iniciar gameplay. */
        g_ship.descendStartPos = (Vector3){ landX, g_ship.startHeight, landZ - 30.0f };
        g_ship.position = g_ship.descendStartPos;
    }
    /* F6: inicia movimento da nave em direção ao norte (+Z); só quando já pousou. */
    if (g_useStreamingWorld && g_voxelWorld && IsKeyPressed(KEY_F6) && g_ship.state == SHIP_HOVER_READY) {
        g_ship.speed = 1.5f;
        g_ship.moveDirX = 0.0f;
        g_ship.moveDirZ = 1.0f;
    }
    if (g_fpsCamera.locked) {
        float moveSpeed = 5.0f * (KeyDown(ACT_SPRINT) ? 1.6f : 1.0f);

        if (g_noClip) {
            float forward = 0.0f, right = 0.0f;
            if (KeyDown(ACT_MOVE_FORWARD)) forward += 1.0f;
            if (KeyDown(ACT_MOVE_BACK)) forward -= 1.0f;
            if (KeyDown(ACT_MOVE_LEFT)) right -= 1.0f;
            if (KeyDown(ACT_MOVE_RIGHT)) right += 1.0f;
            
            // Normaliza movimento diagonal
            if (forward != 0.0f || right != 0.0f) {
                float len = sqrtf(forward * forward + right * right);
                if (len > 0.0001f) {
                    forward /= len;
                    right /= len;
                }
            }
            
            // Obtém vetores forward e right da câmera no plano XZ
            float forwardX, forwardZ;
            float rightX, rightZ;
            FPSCamera_GetForwardFlat(&g_fpsCamera, &forwardX, &forwardZ);
            FPSCamera_GetRightFlat(&g_fpsCamera, &rightX, &rightZ);
            
            float moveX = (forwardX * forward + rightX * right) * moveSpeed * dt;
            float moveZ = (forwardZ * forward + rightZ * right) * moveSpeed * dt;
            g_playerPhysics.x += moveX;
            g_playerPhysics.z += moveZ;
            float moveY = 0.0f;
            if (IsKeyDown(KEY_SPACE)) moveY += 1.0f;
            if (IsKeyDown(KEY_LEFT_CONTROL)) moveY -= 1.0f;
            if (IsKeyDown(KEY_Q)) moveY += 1.0f;
            if (IsKeyDown(KEY_E)) moveY -= 1.0f;
            g_playerPhysics.y += moveY * moveSpeed * dt;
            
            // NO CLIPPING: Não aplica colisão
        } else {
            float forward = 0.0f, right = 0.0f;
            if (KeyDown(ACT_MOVE_FORWARD)) forward += 1.0f;
            if (KeyDown(ACT_MOVE_BACK)) forward -= 1.0f;
            if (KeyDown(ACT_MOVE_LEFT)) right -= 1.0f;
            if (KeyDown(ACT_MOVE_RIGHT)) right += 1.0f;
            if (forward != 0.0f || right != 0.0f) {
                float len = sqrtf(forward * forward + right * right);
                if (len > 0.0001f) { forward /= len; right /= len; }
            }
            float forwardX, forwardZ, rightX, rightZ;
            FPSCamera_GetForwardFlat(&g_fpsCamera, &forwardX, &forwardZ);
            FPSCamera_GetRightFlat(&g_fpsCamera, &rightX, &rightZ);
            float moveX = (forwardX * forward + rightX * right) * moveSpeed * dt;
            float moveZ = (forwardZ * forward + rightZ * right) * moveSpeed * dt;
            if (g_isOnLadder) { moveX = 0.0f; moveZ = 0.0f; } /* escada: só sobe, centraliza no X depois. */
            // Movimento e colisão por eixo (X → Z → Y) para não escalar parede nem grudar no chão
            g_playerPhysics.x += moveX;
            ResolveCollisionsX();
            g_playerPhysics.z += moveZ;
            ResolveCollisionsZ();
            
            // Verifica ground ANTES do pulo: checa se há bloco sólido abaixo dos pés
            // Esta verificação é feita ANTES do movimento Y para garantir que o pulo funcione
            bool isOnGround = false;
            
            // Primeiro, verifica se estava no chão no frame anterior (da colisão)
            // Isso ajuda a detectar ground mesmo quando o player está parado
            if (g_playerPhysics.onGround && g_playerPhysics.vy <= 0.1f) {
                isOnGround = true;
            }
            
            // Verifica múltiplos pontos abaixo dos pés para melhor detecção
            int32_t feetX = (int32_t)floorf(g_playerPhysics.x);
            int32_t feetY = (int32_t)floorf(g_playerPhysics.y) - 1; // Um bloco abaixo dos pés
            int32_t feetZ = (int32_t)floorf(g_playerPhysics.z);
            
            // Verifica se há bloco sólido abaixo e se o player está próximo o suficiente
            if (IsBlockSolid(feetX, feetY, feetZ)) {
                float blockTop = (float)feetY + 1.0f;
                // Player está no chão se:
                // 1. Está dentro de uma tolerância do topo do bloco
                // 2. E não está caindo muito rápido (velocidade Y pequena ou zero)
                if (g_playerPhysics.y >= blockTop - 0.15f && g_playerPhysics.y <= blockTop + 0.25f) {
                    if (g_playerPhysics.vy <= 0.1f) { // Não está caindo ou caindo muito devagar
                        isOnGround = true;
                    }
                }
            }
            
            if (KeyPressed(ACT_JUMP) && isOnGround) {
                g_playerPhysics.vy = JUMP_FORCE;
                isOnGround = false; // Não está mais no chão após pular
                g_playerPhysics.onGround = false; // Reseta flag também
            }

            /* Escada: sobe automaticamente, gravidade desativada, centraliza no X. */
            if (g_isOnLadder) {
                g_playerPhysics.vx = 0.0f;
                g_playerPhysics.vz = 0.0f;
                g_playerPhysics.vy = 2.0f; /* velocidade de subida */
                float ladderCenterX = (g_ship.ladderBox.min.x + g_ship.ladderBox.max.x) * 0.5f;
                g_playerPhysics.x = LerpFloat(g_playerPhysics.x, ladderCenterX, 6.0f * dt);
            } else {
                /* Gravidade quando não está no chão. */
                if (!isOnGround) {
                    g_playerPhysics.vy += GRAVITY * dt;
                    if (g_playerPhysics.vy < MAX_FALL_SPEED) {
                        g_playerPhysics.vy = MAX_FALL_SPEED;
                    }
                } else {
                    g_playerPhysics.vy = 0.0f;
                }
            }
            
            // Atualiza posição Y com velocidade e resolve colisão só no eixo Y
            g_playerPhysics.y += g_playerPhysics.vy * dt;
            ResolveCollisionsY();
            
            // Após colisão, se estiver no chão, garante que velocidade Y seja zero
            if (g_playerPhysics.onGround) {
                g_playerPhysics.vy = 0.0f;
                // Garante que está colidindo quando está no chão
                if (g_isColliding) {
                    // Já está marcado como colidindo
                }
            }
        }
    }
    
    if (g_arcTerminalFull) {
        ArcTerminalFull_Update(g_arcTerminalFull, dt);
    }
    if (g_useStreamingWorld && g_voxelWorld) {
        WorldBeware_Update(&g_worldBeware, g_ship.position.x, g_ship.position.z, 0.0f, g_playerPhysics.x, g_playerPhysics.z);
        /* Zona segura (deck): jogador dentro = isSafe (oxigênio, etc.). */
        {
            Vector3 pMin = {
                g_playerPhysics.x - g_playerPhysics.width * 0.5f,
                g_playerPhysics.y,
                g_playerPhysics.z - g_playerPhysics.depth * 0.5f
            };
            Vector3 pMax = {
                g_playerPhysics.x + g_playerPhysics.width * 0.5f,
                g_playerPhysics.y + g_playerPhysics.height,
                g_playerPhysics.z + g_playerPhysics.depth * 0.5f
            };
            BoundingBox playerBox = (BoundingBox){ pMin, pMax };
            g_playerPhysics.isSafe = CheckCollisionBoxes(playerBox, g_ship.deckBox);
        }
    } else {
        g_playerPhysics.isSafe = false;
    }
    
    // Atualiza posição da câmera para seguir o player
    g_fpsCamera.position.x = g_playerPhysics.x;
    g_fpsCamera.position.y = g_playerPhysics.y + PLAYER_EYE_HEIGHT;  /* y = pé; câmera nos olhos. */
    g_fpsCamera.position.z = g_playerPhysics.z;

    /* Head bob só enquanto segura teclas de movimento; desliga ao parar. Desativado durante pouso. */
    {
        bool allowBob = (g_ship.state == SHIP_HOVER_READY);
        int moving = 0;
        if (allowBob && !g_noClip && (KeyDown(ACT_MOVE_FORWARD) || KeyDown(ACT_MOVE_BACK) || KeyDown(ACT_MOVE_LEFT) || KeyDown(ACT_MOVE_RIGHT)))
            moving = 1;
        if (!allowBob || !moving) {
            g_fpsCamera.bobOffset.x = g_fpsCamera.bobOffset.y = g_fpsCamera.bobOffset.z = 0.0f;
            g_walkBobPhase = 0.0f;
        } else {
            float bobSpeed = 8.0f * (KeyDown(ACT_SPRINT) ? 1.4f : 1.0f);
            g_walkBobPhase += dt * bobSpeed;
            float bobY = 0.048f * sinf(g_walkBobPhase);
            float sway = 0.022f * sinf(g_walkBobPhase * 2.0f);
            float rx, rz;
            FPSCamera_GetRightFlat(&g_fpsCamera, &rx, &rz);
            g_fpsCamera.bobOffset.x = rx * sway;
            g_fpsCamera.bobOffset.y = bobY;
            g_fpsCamera.bobOffset.z = rz * sway;
        }
    }

    // Input/rede: movimento é via WASD + PhysicsBody. InputCmd reservado para rede.
    (void)Input_GetCommand(); /* uso futuro: enviar PKT_INPUT ao host */
}

/* Slider genérico: t em [0,1]. bg=fundo, fill=preenchimento, knob=controle. */
static void DrawSlider(int x, int y, int w, int h, float t, Color bg, Color fill, Color knob) {
    DrawRectangle(x, y, w, h, bg);
    int fillW = (int)(w * t);
    if (fillW > 0) DrawRectangle(x, y, fillW, h, fill);
    int kx = x + fillW - 4;
    DrawRectangle(kx, y - 4, 8, h + 8, knob);
}

static void DrawCrosshairAt(int cx, int cy) {
    if (g_settings.crosshair == CROSSHAIR_DOT) {
        DrawCircle(cx, cy, 2, RAYWHITE);
        DrawCircleLines(cx, cy, 4, Fade(RAYWHITE, 0.35f));
    } else {
        int gap = 2, len = 6;
        DrawLine(cx - len, cy, cx - gap, cy, RAYWHITE);
        DrawLine(cx + gap, cy, cx + len, cy, RAYWHITE);
        DrawLine(cx, cy - len, cx, cy - gap, RAYWHITE);
        DrawLine(cx, cy + gap, cx, cy + len, RAYWHITE);
        DrawCircleLines(cx, cy, 8, Fade(RAYWHITE, 0.2f));
    }
}

static void DrawCrosshair(void) {
    DrawCrosshairAt(GetScreenWidth() / 2, GetScreenHeight() / 2);
}

void Scene_Gameplay_Draw(void) {
    if (!g_initialized) return;

    /* Terminal ARC: renderiza no RT próprio somente quando aberto. */
    if (g_arcTerminalFull && ArcTerminalFull_IsOpen(g_arcTerminalFull)) {
        ArcTerminalFull_Render(g_arcTerminalFull);
    }

    BeginTextureMode(g_crtTarget);
    ClearBackground(BLACK);

    // ——— 2D (antes do 3D): só fallback do céu. Nunca desenhar UI/menu aqui. ———
    if (g_skyShader.id == 0) {
        Atmosphere_DrawSky(&g_atmosphere);
    }
#if !defined(USE_RLGL)
    else {
        Atmosphere_DrawSky(&g_atmosphere); /* sem rlgl: não desenha esfera 3D */
    }
#endif
#if defined(USE_RLGL)
    rlDrawRenderBatchActive();  // flush 2D antes de BeginMode3D; evita vértices 2D com projeção 3D
#endif

    // ——— 3D: sky (esfera), grid, blocos. Tudo entre BeginMode3D e EndMode3D. ———
    Camera3D raylibCam = FPSCamera_GetRaylibCamera(&g_fpsCamera);
    BeginMode3D(raylibCam);

#if defined(USE_RLGL)
    // Checklist sky: flush antes; estados culling/depth; DrawSphere; restaurar; flush depois.
    if (g_skyShader.id != 0 && g_skyLocBottom >= 0 && g_skyLocHorizon >= 0 && g_skyLocTop >= 0) {
        rlDrawRenderBatchActive();          // antes do sky: não desenhar lixo com estados alterados
        rlDisableBackfaceCulling();
        rlDisableDepthMask();
        BeginShaderMode(g_skyShader);
        float sb[3] = { g_atmosphere.sky.bottomColor.r/255.0f, g_atmosphere.sky.bottomColor.g/255.0f, g_atmosphere.sky.bottomColor.b/255.0f };
        float sh[3] = { g_atmosphere.sky.horizonColor.r/255.0f, g_atmosphere.sky.horizonColor.g/255.0f, g_atmosphere.sky.horizonColor.b/255.0f };
        float st[3] = { g_atmosphere.sky.topColor.r/255.0f, g_atmosphere.sky.topColor.g/255.0f, g_atmosphere.sky.topColor.b/255.0f };
        SetShaderValue(g_skyShader, g_skyLocBottom, sb, SHADER_UNIFORM_VEC3);
        SetShaderValue(g_skyShader, g_skyLocHorizon, sh, SHADER_UNIFORM_VEC3);
        SetShaderValue(g_skyShader, g_skyLocTop, st, SHADER_UNIFORM_VEC3);
        DrawSphere(g_fpsCamera.position, 400.0f, WHITE);
        EndShaderMode();
        rlEnableDepthMask();
        rlEnableBackfaceCulling();
        rlDrawRenderBatchActive();          // depois do sky: isolado do grid/blocos
    }
#endif

    if (g_dbgShowGrid) {
        int32_t gridSize = 60;
        int32_t gridSpacing = 10;
        Color gridColor = (Color){50, 50, 50, 100};
        for (int z = -gridSize; z <= gridSize; z += gridSpacing) {
            float zf = (float)z;
            if (!isfinite((double)zf)) continue;
            DrawLine3D((Vector3){(float)-gridSize, 0.5f, zf}, (Vector3){(float)gridSize, 0.5f, zf}, gridColor);
        }
        for (int x = -gridSize; x <= gridSize; x += gridSpacing) {
            float xf = (float)x;
            if (!isfinite((double)xf)) continue;
            DrawLine3D((Vector3){xf, 0.5f, (float)-gridSize}, (Vector3){xf, 0.5f, (float)gridSize}, gridColor);
        }
#if defined(USE_RLGL)
        rlDrawRenderBatchActive();
#endif
    }

    // Fog no forward: view-space d = -viewPos.z. Shader só em blocos (triângulos + wireframe).
    if (g_atmosphere.fog.enabled && g_fogShader.id != 0 && g_fogLocStart >= 0 && g_fogLocEnd >= 0 && g_fogLocColor >= 0) {
        BeginShaderMode(g_fogShader);
        float fogStart = g_atmosphere.fog.startDistance;
        float fogEnd = g_atmosphere.fog.endDistance;
        float fogColorVec3[3] = {
            g_atmosphere.fog.fogColor.r / 255.0f,
            g_atmosphere.fog.fogColor.g / 255.0f,
            g_atmosphere.fog.fogColor.b / 255.0f
        };
        SetShaderValue(g_fogShader, g_fogLocStart, &fogStart, SHADER_UNIFORM_FLOAT);
        SetShaderValue(g_fogShader, g_fogLocEnd, &fogEnd, SHADER_UNIFORM_FLOAT);
        SetShaderValue(g_fogShader, g_fogLocColor, fogColorVec3, SHADER_UNIFORM_VEC3);
        if (g_fogLocType >= 0) {
            int fogType = (int)g_atmosphere.fog.type;
            SetShaderValue(g_fogShader, g_fogLocType, &fogType, SHADER_UNIFORM_INT);
        }
        if (g_fogLocDensity >= 0) {
            float fogDensity = (g_atmosphere.fog.type == FOG_EXPONENTIAL)
                ? (-logf(1.0f - FOG_EXP_TARGET) / g_atmosphere.fog.endDistance)
                : g_atmosphere.fog.density;
            SetShaderValue(g_fogShader, g_fogLocDensity, &fogDensity, SHADER_UNIFORM_FLOAT);
        }
        if (g_fogLocHorizon >= 0) {
            float fh[3] = { g_atmosphere.fog.fogColor.r/255.0f, g_atmosphere.fog.fogColor.g/255.0f, g_atmosphere.fog.fogColor.b/255.0f };
            SetShaderValue(g_fogShader, g_fogLocHorizon, fh, SHADER_UNIFORM_VEC3);
        }
        if (g_fogLocSky >= 0) {
            float fs[3] = { g_atmosphere.sky.topColor.r/255.0f, g_atmosphere.sky.topColor.g/255.0f, g_atmosphere.sky.topColor.b/255.0f };
            SetShaderValue(g_fogShader, g_fogLocSky, fs, SHADER_UNIFORM_VEC3);
        }
        if (g_fogLocH0 >= 0) {
            float fogH0 = 0.0f;
            SetShaderValue(g_fogShader, g_fogLocH0, &fogH0, SHADER_UNIFORM_FLOAT);
        }
        if (g_fogLocHRange >= 0) {
            float fogHRange = 20.0f;
            SetShaderValue(g_fogShader, g_fogLocHRange, &fogHRange, SHADER_UNIFORM_FLOAT);
        }
    }

    // ——— Passe 1: só triângulos (faces sólidas), com fog shader ———
    const float RENDER_DISTANCE = 30.0f;
    const float RENDER_DISTANCE_SQ = RENDER_DISTANCE * RENDER_DISTANCE;
    int32_t playerBlockX = (int32_t)floorf(g_playerPhysics.x);
    int32_t playerBlockZ = (int32_t)floorf(g_playerPhysics.z);
    int32_t renderRadius = (int32_t)RENDER_DISTANCE + 5;
    int facesInBatch = 0;

    for (int32_t x = playerBlockX - renderRadius; x <= playerBlockX + renderRadius; x++) {
        for (int32_t z = playerBlockZ - renderRadius; z <= playerBlockZ + renderRadius; z++) {
            float dx = (float)x + 0.5f - g_playerPhysics.x;
            float dz = (float)z + 0.5f - g_playerPhysics.z;
            if (dx * dx + dz * dz > RENDER_DISTANCE_SQ) continue;
            for (int32_t y = 0; y < MAP_SIZE_Y; y++) {
                if (!IsBlockSolid(x, y, z)) continue;
                Color blockColor;
                if (g_useStreamingWorld && g_voxelWorld) {
                    Voxel v = VoxelWorld_GetBlock(g_voxelWorld, x, y, z);
                    switch (v.type) {
                        case BLOCK_TERRAIN: blockColor = (Color){101, 67, 33, 255}; break;
                        case BLOCK_BLACK:   blockColor = (Color){20, 20, 20, 255}; break;
                        case BLOCK_GRAY:   blockColor = (Color){128, 128, 128, 255}; break;
                        case BLOCK_RED:    blockColor = (Color){255, 50, 50, 255}; break;
                        case BLOCK_ORANGE: blockColor = (Color){200, 100, 50, 255}; break;
                        case BLOCK_GREEN:  blockColor = (Color){50, 180, 80, 255}; break;
                        case BLOCK_PURPLE:
                        case BLOCK_VIOLET: blockColor = (Color){120, 80, 180, 255}; break;
                        default: blockColor = (Color){128, 128, 128, 255}; break;
                    }
                } else {
                    switch (g_map[x + MAP_OFFSET_X][y][z + MAP_OFFSET_Z]) {
                        case BLOCK_TERRAIN: blockColor = (Color){101, 67, 33, 255}; break;
                        case BLOCK_GRAY:   blockColor = (Color){128, 128, 128, 255}; break;
                        case BLOCK_RED:    blockColor = (Color){255, 50, 50, 255}; break;
                        default: continue;
                    }
                }
                if (!IsBlockSolid(x - 1, y, z)) DrawBlockFace_Solid((float)x, (float)y, (float)z, 0, blockColor, &facesInBatch);
                if (!IsBlockSolid(x + 1, y, z)) DrawBlockFace_Solid((float)x, (float)y, (float)z, 1, blockColor, &facesInBatch);
                if (!IsBlockSolid(x, y - 1, z)) DrawBlockFace_Solid((float)x, (float)y, (float)z, 2, blockColor, &facesInBatch);
                if (!IsBlockSolid(x, y + 1, z)) DrawBlockFace_Solid((float)x, (float)y, (float)z, 3, blockColor, &facesInBatch);
                if (!IsBlockSolid(x, y, z - 1)) DrawBlockFace_Solid((float)x, (float)y, (float)z, 4, blockColor, &facesInBatch);
                if (!IsBlockSolid(x, y, z + 1)) DrawBlockFace_Solid((float)x, (float)y, (float)z, 5, blockColor, &facesInBatch);
            }
        }
    }

    if (g_useStreamingWorld && g_voxelWorld) {
        Ship_Draw(&g_ship);
    }

    if (g_atmosphere.fog.enabled && g_fogShader.id != 0) {
        EndShaderMode();
    }
#if defined(USE_RLGL)
    rlDrawRenderBatchActive();
#endif

        /* Monitor na nave: cubo cinza (terminal físico); interação por proximidade. */
        if (g_useStreamingWorld && g_voxelWorld) {
            float deckTop = g_ship.hullBox.max.y;
            float monY = deckTop + 0.5f;
            float monZ = g_ship.position.z + 1.5f;
            DrawCube((Vector3){g_ship.position.x, monY, monZ}, 0.3f, 1.0f, 0.3f, (Color){50, 55, 60, 255});
        }
#if defined(USE_RLGL)
    rlDrawRenderBatchActive();  // flush sólidos/fog antes do passe de wireframe
#endif

    // ——— Passe 2: só linhas (wireframe), shader padrão, fora do fog. F4 = g_dbgShowWireframe. ———
    if (g_dbgShowWireframe) {
    int linesInBatch = 0;
    for (int32_t x = playerBlockX - renderRadius; x <= playerBlockX + renderRadius; x++) {
        for (int32_t z = playerBlockZ - renderRadius; z <= playerBlockZ + renderRadius; z++) {
            float dx = (float)x + 0.5f - g_playerPhysics.x;
            float dz = (float)z + 0.5f - g_playerPhysics.z;
            if (dx * dx + dz * dz > RENDER_DISTANCE_SQ) continue;
            for (int32_t y = 0; y < MAP_SIZE_Y; y++) {
                if (!IsBlockSolid(x, y, z)) continue;
                if (!IsBlockSolid(x - 1, y, z)) DrawBlockFace_Wire((float)x, (float)y, (float)z, 0, &linesInBatch);
                if (!IsBlockSolid(x + 1, y, z)) DrawBlockFace_Wire((float)x, (float)y, (float)z, 1, &linesInBatch);
                if (!IsBlockSolid(x, y - 1, z)) DrawBlockFace_Wire((float)x, (float)y, (float)z, 2, &linesInBatch);
                if (!IsBlockSolid(x, y + 1, z)) DrawBlockFace_Wire((float)x, (float)y, (float)z, 3, &linesInBatch);
                if (!IsBlockSolid(x, y, z - 1)) DrawBlockFace_Wire((float)x, (float)y, (float)z, 4, &linesInBatch);
                if (!IsBlockSolid(x, y, z + 1)) DrawBlockFace_Wire((float)x, (float)y, (float)z, 5, &linesInBatch);
            }
        }
    }
    /* Nave: wire + 8 vértices do AABB (só debug visual). */
    {
        const BoundingBox* bb = &g_ship.hullBox;
        DrawBoundingBox(*bb, LIME);
        const float r = 0.05f;
        Vector3 c[] = {
            { bb->min.x, bb->min.y, bb->min.z }, { bb->max.x, bb->min.y, bb->min.z },
            { bb->min.x, bb->max.y, bb->min.z }, { bb->max.x, bb->max.y, bb->min.z },
            { bb->min.x, bb->min.y, bb->max.z }, { bb->max.x, bb->min.y, bb->max.z },
            { bb->min.x, bb->max.y, bb->max.z }, { bb->max.x, bb->max.y, bb->max.z }
        };
        for (int i = 0; i < 8; i++) DrawSphere(c[i], r, LIME);
    }
#if defined(USE_RLGL)
        rlDrawRenderBatchActive();
#endif
    }

    EndMode3D();

#if defined(USE_RLGL)
    rlDrawRenderBatchActive();  // transição 3D→2D: evita vértices 3D desenhados com projeção 2D
#endif

    // ——— 2D (sempre depois de EndMode3D): HUD ou overlay de pause (centralizado, fonte assets/fonts). ———
    if (g_mode == GP_PAUSED) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.55f));
        float cx = (float)GetScreenWidth() / 2.0f;
        float cy = (float)GetScreenHeight() / 2.0f;
        float titleFont = 32.0f;
        float optionFont = 20.0f;
        float smallFont = 18.0f;
        char buf[80];
        if (g_waitingForKey) {
            const char* t = "CONTROLS";
            Vector2 tw = MeasureTextEx(g_consolaFont, t, titleFont, 0.0f);
            DrawTextEx(g_consolaFont, t, (Vector2){cx - tw.x / 2.0f, cy - 60.0f}, titleFont, 0.0f, RAYWHITE);
            const char* msg = "Press a key... ESC cancels";
            Vector2 mw = MeasureTextEx(g_consolaFont, msg, optionFont, 0.0f);
            DrawTextEx(g_consolaFont, msg, (Vector2){cx - mw.x / 2.0f, cy - 10.0f}, optionFont, 0.0f, RAYWHITE);
        } else if (g_pausePage == PAUSE_MAIN) {
            Vector2 pw = MeasureTextEx(g_consolaFont, "PAUSED", titleFont, 0.0f);
            DrawTextEx(g_consolaFont, "PAUSED", (Vector2){cx - pw.x / 2.0f, cy - 100.0f}, titleFont, 0.0f, RAYWHITE);
            static const char* mainOpts[] = { "RESUME", "SETTINGS", "EXIT TO MENU" };
            float startY = cy - 36.0f;
            float rowH = 40.0f;
            for (int i = 0; i < 3; i++) {
                const char* label = mainOpts[i];
                Color optColor = (g_pauseSelectionMain == i) ? RAYWHITE : Fade(RAYWHITE, 0.75f);
                Vector2 ow = MeasureTextEx(g_consolaFont, label, optionFont, 0.0f);
                float oy = startY + (float)i * rowH;
                DrawTextEx(g_consolaFont, label, (Vector2){cx - ow.x / 2.0f, oy}, optionFont, 0.0f, optColor);
            }
        } else if (g_pausePage == PAUSE_SETTINGS) {
            Vector2 sw = MeasureTextEx(g_consolaFont, "SETTINGS", titleFont, 0.0f);
            DrawTextEx(g_consolaFont, "SETTINGS", (Vector2){cx - sw.x / 2.0f, cy - 140.0f}, titleFont, 0.0f, RAYWHITE);
            float startY = cy - 100.0f;
            const float rowH = 44.0f;
            const float panelW = 420.0f;
            float y = startY;

            /* FOV */
            if (g_pauseSelectionSettings == SET_FOV) DrawRectangle((int)(cx - panelW/2), (int)(y - 6), (int)panelW, (int)rowH, Fade(RAYWHITE, 0.08f));
            DrawTextEx(g_consolaFont, "FOV", (Vector2){cx - panelW/2 + 20, y}, optionFont, 0.0f, RAYWHITE);
            snprintf(buf, sizeof(buf), "%.0f", g_settings.fov);
            Vector2 vw = MeasureTextEx(g_consolaFont, buf, optionFont, 0.0f);
            DrawTextEx(g_consolaFont, buf, (Vector2){cx + panelW/2 - 20 - vw.x, y}, optionFont, 0.0f, RAYWHITE);
            { float fovT = (g_uiAnim.fovVis - 60.0f) / (110.0f - 60.0f); fovT = Clamp(fovT, 0.0f, 1.0f); float pulse = 0.5f + 0.5f * sinf(GetTime() * 6.0f); Color knob = (g_pauseSelectionSettings == SET_FOV) ? Fade(RAYWHITE, 0.7f + 0.3f * pulse) : RAYWHITE; DrawSlider((int)(cx - 100), (int)(y + 24), 200, 8, fovT, (Color){40,40,40,255}, (Color){200,200,200,255}, knob); }
            y += rowH;

            /* SENSITIVITY */
            if (g_pauseSelectionSettings == SET_SENS) DrawRectangle((int)(cx - panelW/2), (int)(y - 6), (int)panelW, (int)rowH, Fade(RAYWHITE, 0.08f));
            DrawTextEx(g_consolaFont, "SENSITIVITY", (Vector2){cx - panelW/2 + 20, y}, optionFont, 0.0f, RAYWHITE);
            snprintf(buf, sizeof(buf), "%.2f", g_settings.mouseSensitivity);
            vw = MeasureTextEx(g_consolaFont, buf, optionFont, 0.0f);
            DrawTextEx(g_consolaFont, buf, (Vector2){cx + panelW/2 - 20 - vw.x, y}, optionFont, 0.0f, RAYWHITE);
            { float sensT = (g_uiAnim.sensVis - 0.05f) / (1.0f - 0.05f); sensT = Clamp(sensT, 0.0f, 1.0f); float pulse = 0.5f + 0.5f * sinf(GetTime() * 6.0f); Color knob = (g_pauseSelectionSettings == SET_SENS) ? Fade(RAYWHITE, 0.7f + 0.3f * pulse) : RAYWHITE; DrawSlider((int)(cx - 100), (int)(y + 24), 200, 8, sensT, (Color){40,40,40,255}, (Color){200,200,200,255}, knob); }
            y += rowH;

            /* CROSSHAIR */
            if (g_pauseSelectionSettings == SET_CROSSHAIR) DrawRectangle((int)(cx - panelW/2), (int)(y - 6), (int)panelW, (int)rowH, Fade(RAYWHITE, 0.08f));
            const char* crossStr = g_settings.crosshair == CROSSHAIR_DOT ? "CROSSHAIR: DOT" : "CROSSHAIR: PLUS";
            Vector2 cw = MeasureTextEx(g_consolaFont, crossStr, optionFont, 0.0f);
            DrawTextEx(g_consolaFont, crossStr, (Vector2){cx - cw.x / 2.0f, y}, optionFont, 0.0f, RAYWHITE);
            if (g_pauseSelectionSettings == SET_CROSSHAIR) DrawCrosshairAt((int)(cx + panelW/2 - 50), (int)(y + 28));
            y += rowH;

            /* CONTROLS */
            if (g_pauseSelectionSettings == SET_CONTROLS) DrawRectangle((int)(cx - panelW/2), (int)(y - 6), (int)panelW, (int)rowH, Fade(RAYWHITE, 0.08f));
            Vector2 ctrlw = MeasureTextEx(g_consolaFont, "CONTROLS", optionFont, 0.0f);
            DrawTextEx(g_consolaFont, "CONTROLS", (Vector2){cx - ctrlw.x / 2.0f, y}, optionFont, 0.0f, RAYWHITE);
            y += rowH;

            /* BACK */
            if (g_pauseSelectionSettings == SET_BACK) DrawRectangle((int)(cx - panelW/2), (int)(y - 6), (int)panelW, (int)rowH, Fade(RAYWHITE, 0.08f));
            Vector2 backw = MeasureTextEx(g_consolaFont, "BACK", optionFont, 0.0f);
            DrawTextEx(g_consolaFont, "BACK", (Vector2){cx - backw.x / 2.0f, y}, optionFont, 0.0f, RAYWHITE);
        } else {
            Vector2 cw = MeasureTextEx(g_consolaFont, "CONTROLS", titleFont, 0.0f);
            DrawTextEx(g_consolaFont, "CONTROLS", (Vector2){cx - cw.x / 2.0f, cy - 120.0f}, titleFont, 0.0f, RAYWHITE);
            float startY = cy - 80.0f;
            float rowH = 26.0f;
            float y = startY;
            for (int i = 0; i < ACT_COUNT; i++) {
                if (g_pauseSelectionBinds == i) DrawRectangle((int)(cx - 210), (int)(y - 2), 420, (int)(rowH + 4), Fade(RAYWHITE, 0.08f));
                snprintf(buf, sizeof(buf), "%s: %s", Settings_ActionName((ActionId)i), Settings_KeyName(g_settings.keys[i].primary));
                Vector2 bw = MeasureTextEx(g_consolaFont, buf, smallFont, 0.0f);
                Color rowColor = (g_pauseSelectionBinds == i) ? RAYWHITE : Fade(RAYWHITE, 0.85f);
                DrawTextEx(g_consolaFont, buf, (Vector2){cx - bw.x / 2.0f, y}, smallFont, 0.0f, rowColor);
                y += rowH;
            }
            if (g_pauseSelectionBinds == ACT_COUNT) DrawRectangle((int)(cx - 210), (int)(y - 2), 420, (int)(rowH + 4), Fade(RAYWHITE, 0.08f));
            Vector2 backw = MeasureTextEx(g_consolaFont, "BACK", optionFont, 0.0f);
            DrawTextEx(g_consolaFont, "BACK", (Vector2){cx - backw.x / 2.0f, y}, optionFont, 0.0f, (g_pauseSelectionBinds == ACT_COUNT) ? RAYWHITE : Fade(RAYWHITE, 0.85f));
        }
    } else {
        if (g_dbgShowStats) {
            float fontSize = 16.0f, lineHeight = 20.0f, startY = 10.0f;
            Color textColor = WHITE;
            char info[256];
            DrawFPS(10, (int)startY);
            startY += 25.0f;
            snprintf(info, sizeof(info), "Position: X=%.1f Y=%.1f Z=%.1f", g_playerPhysics.x, g_playerPhysics.y, g_playerPhysics.z);
            DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
            startY += lineHeight;
            snprintf(info, sizeof(info), "Velocity: X=%.1f Y=%.1f Z=%.1f", g_playerPhysics.vx, g_playerPhysics.vy, g_playerPhysics.vz);
            DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
            startY += lineHeight;
            bool isColliding = g_isColliding && !g_noClip;
            snprintf(info, sizeof(info), "Collision: %s", isColliding ? "YES" : "NO");
            DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
            startY += lineHeight;
            snprintf(info, sizeof(info), "Standing: %d  OnGround: %d", g_standingOnShip ? 1 : 0, g_playerPhysics.onGround ? 1 : 0);
            DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
            startY += lineHeight;
            if (g_useStreamingWorld && g_voxelWorld) {
                int32_t minZ = 0, maxZ = 0;
                WorldBeware_GetStreamRange(&g_worldBeware, &minZ, &maxZ);
                snprintf(info, sizeof(info), "STREAM Z: %d..%d", minZ, maxZ);
                DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
                startY += lineHeight;
                snprintf(info, sizeof(info), "Safe (deck): %s", g_playerPhysics.isSafe ? "YES" : "NO");
                DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
                startY += lineHeight;
                const char* shipStateStr = "?";
                if (g_ship.state == SHIP_IDLE_HOVER) shipStateStr = "IDLE_HOVER";
                else if (g_ship.state == SHIP_DESCENDING) shipStateStr = "DESCENDING";
                else if (g_ship.state == SHIP_HOVER_READY) shipStateStr = "HOVER_READY";
                snprintf(info, sizeof(info), "ShipState: %s  ShipY: %.1f  ShipSpeed: %.1f", shipStateStr, g_ship.position.y, g_ship.speed);
                DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
                startY += lineHeight;
                snprintf(info, sizeof(info), "ShipY: %.1f  HullMinY: %.1f  HullMaxY: %.1f", g_ship.position.y, g_ship.hullBox.min.y, g_ship.hullBox.max.y);
                DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
                startY += lineHeight;
                float playerFootY = g_playerPhysics.y;
                float playerCenterY = g_playerPhysics.y + g_playerPhysics.height * 0.5f;
                float playerHeadY = g_playerPhysics.y + g_playerPhysics.height;
                snprintf(info, sizeof(info), "PlayerFootY: %.1f  PlayerCenterY: %.1f  PlayerHeadY: %.1f", playerFootY, playerCenterY, playerHeadY);
                DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
                startY += lineHeight;
            }
            if (g_useStreamingWorld && g_voxelWorld) {
                int32_t loadedChunks = 0;
                VoxelWorld_GetStats(g_voxelWorld, &loadedChunks, NULL);
                snprintf(info, sizeof(info), "Chunks: %d", loadedChunks);
            } else {
                int32_t blockCount = 0;
                for (int32_t x = 0; x < MAP_SIZE_X; x++)
                    for (int32_t y = 0; y < MAP_SIZE_Y; y++)
                        for (int32_t z = 0; z < MAP_SIZE_Z; z++)
                            if (g_map[x][y][z] != BLOCK_AIR) blockCount++;
                snprintf(info, sizeof(info), "Map Blocks: %d", blockCount);
            }
            DrawTextEx(g_consolaFont, info, (Vector2){10.0f, startY}, fontSize, 0.0f, textColor);
        }
        if (g_showDirection)
            Debug_DrawCompass(&g_fpsCamera);
        if (g_nearArcMonitor && !ArcTerminalFull_IsOpen(g_arcTerminalFull)) {
            const char* hint = "Press E to open Terminal";
            Vector2 hs = MeasureTextEx(g_consolaFont, hint, 18, 1);
            float hx = ((float)g_crtTarget.texture.width - hs.x) / 2.0f;
            float hy = (float)g_crtTarget.texture.height * 0.15f;
            DrawRectangle((int)(hx - 12), (int)(hy - 4), (int)(hs.x + 24), (int)(hs.y + 12), (Color){0, 0, 0, 180});
            DrawTextEx(g_consolaFont, hint, (Vector2){hx, hy}, 18, 1, (Color){51, 255, 51, 255});
        }
        DrawCrosshair();
        DrawText("ESC - Pause", 20, GetScreenHeight() - 30, 16, Fade(RAYWHITE, 0.5f));
    }
    EndTextureMode();

    /* CRT por cima da tela (capinha; não altera shaders nem lógica da gameplay) */
    BeginDrawing();
    ClearBackground(BLACK);
    {
        Rectangle srcRect = { 0, 0, (float)g_crtTarget.texture.width, -(float)g_crtTarget.texture.height };
        if (g_crtShader.id != 0) {
            BeginShaderMode(g_crtShader);
            int timeLoc = GetShaderLocation(g_crtShader, "time");
            if (timeLoc >= 0) {
                float t = (float)GetTime();
                SetShaderValue(g_crtShader, timeLoc, &t, SHADER_UNIFORM_FLOAT);
            }
            DrawTextureRec(g_crtTarget.texture, srcRect, (Vector2){0, 0}, WHITE);
            EndShaderMode();
        } else {
            DrawTextureRec(g_crtTarget.texture, srcRect, (Vector2){0, 0}, WHITE);
        }
        /* Terminal ARC overlay: só desenha se terminal aberto E jogo não pausado (não sobrepõe menu ESC). */
        if (g_arcTerminalFull && ArcTerminalFull_IsOpen(g_arcTerminalFull) && g_mode != GP_PAUSED) {
            Texture2D tex = ArcTerminalFull_GetTexture(g_arcTerminalFull);
            if (tex.id != 0) {
                Rectangle ovRect;
                ArcTerminalFull_GetOverlayRect(GetScreenWidth(), GetScreenHeight(), &ovRect);
                Rectangle texSrc = { 0, 0, (float)tex.width, -(float)tex.height };
                if (g_terminalCrtShader.id != 0) {
                    float t = (float)GetTime();
                    if (g_terminalCrtTimeLoc >= 0)
                        SetShaderValue(g_terminalCrtShader, g_terminalCrtTimeLoc, &t, SHADER_UNIFORM_FLOAT);
                    BeginShaderMode(g_terminalCrtShader);
                }
                DrawTextureRec(tex, texSrc, (Vector2){ovRect.x, ovRect.y}, WHITE);
                if (g_terminalCrtShader.id != 0) EndShaderMode();
            }
        }
    }
    EndDrawing();
}
