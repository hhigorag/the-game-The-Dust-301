#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include <raylib.h>
#include <stdbool.h>

// ============================================================================
// ATMOSPHERE - Fog e Sky Gradient
// ============================================================================
// Sistema de atmosfera para profundidade visual sem perder performance.
// Fog leve (linear ou exponencial) e gradient de céu.
// ============================================================================

typedef enum {
    FOG_LINEAR = 0,
    FOG_EXPONENTIAL = 1
} FogType;

typedef struct {
    bool enabled;
    FogType type;
    float startDistance;      // Distância onde o fog começa (linear)
    float endDistance;         // Distância onde o fog termina (linear) ou densidade (exponencial)
    float density;             // Densidade do fog exponencial (0.0 a 1.0)
    Color fogColor;            // Cor do fog (geralmente igual ao sky gradient no horizonte)
} FogSettings;

typedef struct {
    Color topColor;            // Cor do céu no topo (azul claro)
    Color horizonColor;        // Cor do céu no horizonte (azul médio)
    Color bottomColor;         // Cor do céu na parte inferior (azul escuro/roxo)
} SkyGradient;

typedef struct {
    FogSettings fog;
    SkyGradient sky;
    bool initialized;
} Atmosphere;

// Inicializa sistema de atmosfera com valores padrão
void Atmosphere_Init(Atmosphere* atm);

// Limpa recursos do sistema de atmosfera
void Atmosphere_Shutdown(Atmosphere* atm);

// Configura fog
void Atmosphere_SetFog(Atmosphere* atm, bool enabled, FogType type, 
                       float startDist, float endDist, float density, Color fogColor);

// Configura sky gradient
void Atmosphere_SetSkyGradient(Atmosphere* atm, Color top, Color horizon, Color bottom);

// Aplica fog a uma cor baseada na distância da câmera
// Retorna a cor interpolada entre a cor original e a cor do fog
Color Atmosphere_ApplyFog(Atmosphere* atm, Color originalColor, float distanceToCamera);

// Desenha o sky gradient (chamado antes de BeginMode3D)
void Atmosphere_DrawSky(Atmosphere* atm);

// Obtém a cor do fog atual (útil para outras partes do código)
Color Atmosphere_GetFogColor(Atmosphere* atm);

#endif // ATMOSPHERE_H
