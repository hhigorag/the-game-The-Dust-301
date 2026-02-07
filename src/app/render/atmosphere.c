#include "app/render/atmosphere.h"
#include <math.h>

// ============================================================================
// ATMOSPHERE - Implementação
// ============================================================================

void Atmosphere_Init(Atmosphere* atm) {
    if (!atm) return;
    
    // Fog padrão: desabilitado inicialmente
    atm->fog.enabled = false;
    atm->fog.type = FOG_LINEAR;
    atm->fog.startDistance = 20.0f;
    atm->fog.endDistance = 50.0f;
    atm->fog.density = 0.02f;
    atm->fog.fogColor = (Color){20, 20, 30, 255}; // Azul escuro (espaço)
    
    // Sky gradient padrão: azul escuro (espaço)
    atm->sky.topColor = (Color){30, 30, 50, 255};      // Azul escuro no topo
    atm->sky.horizonColor = (Color){20, 20, 30, 255};   // Azul muito escuro no horizonte
    atm->sky.bottomColor = (Color){15, 15, 25, 255};   // Quase preto na parte inferior
    
    atm->initialized = true;
}

void Atmosphere_Shutdown(Atmosphere* atm) {
    if (!atm) return;
    atm->initialized = false;
}

void Atmosphere_SetFog(Atmosphere* atm, bool enabled, FogType type, 
                       float startDist, float endDist, float density, Color fogColor) {
    if (!atm || !atm->initialized) return;
    
    atm->fog.enabled = enabled;
    atm->fog.type = type;
    atm->fog.startDistance = startDist;
    atm->fog.endDistance = endDist;
    atm->fog.density = density;
    atm->fog.fogColor = fogColor;
}

void Atmosphere_SetSkyGradient(Atmosphere* atm, Color top, Color horizon, Color bottom) {
    if (!atm || !atm->initialized) return;
    
    atm->sky.topColor = top;
    atm->sky.horizonColor = horizon;
    atm->sky.bottomColor = bottom;
}

Color Atmosphere_ApplyFog(Atmosphere* atm, Color originalColor, float distanceToCamera) {
    if (!atm || !atm->initialized || !atm->fog.enabled) {
        return originalColor;
    }
    
    float fogFactor = 0.0f;
    
    if (atm->fog.type == FOG_LINEAR) {
        // Fog linear: d já é distância linear (world-space). Nunca use depth
        // não-linear (gl_FragCoord.z/depth buffer) direto — isso gera faixas.
        // fogT = clamp((d - fogStart) / (fogEnd - fogStart), 0, 1)
        float range = atm->fog.endDistance - atm->fog.startDistance;
        fogFactor = (range > 0.0001f)
            ? (distanceToCamera - atm->fog.startDistance) / range
            : 0.0f;
        fogFactor = fmaxf(0.0f, fminf(1.0f, fogFactor));
    } else {
        // Fog exponencial: 1 - e^(-density * distance); d em CPU já é linear
        fogFactor = 1.0f - expf(-atm->fog.density * distanceToCamera);
        fogFactor = fmaxf(0.0f, fminf(1.0f, fogFactor));
    }
    
    // Interpola entre a cor original e a cor do fog
    Color result;
    result.r = (unsigned char)((float)originalColor.r * (1.0f - fogFactor) + (float)atm->fog.fogColor.r * fogFactor);
    result.g = (unsigned char)((float)originalColor.g * (1.0f - fogFactor) + (float)atm->fog.fogColor.g * fogFactor);
    result.b = (unsigned char)((float)originalColor.b * (1.0f - fogFactor) + (float)atm->fog.fogColor.b * fogFactor);
    result.a = originalColor.a;
    
    return result;
}

void Atmosphere_DrawSky(Atmosphere* atm) {
    if (!atm || !atm->initialized) return;
    
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // Desenha gradient vertical do céu
    // Divide a tela em várias faixas horizontais e interpola as cores
    const int gradientSteps = 64; // Número de faixas para o gradient suave
    
    for (int i = 0; i < gradientSteps; i++) {
        float t = (float)i / (float)(gradientSteps - 1); // 0.0 (topo) a 1.0 (fundo)
        
        // Interpola entre as três cores do sky gradient
        Color color;
        if (t < 0.5f) {
            // Primeira metade: topColor -> horizonColor
            float localT = t * 2.0f; // 0.0 a 1.0
            color.r = (unsigned char)((float)atm->sky.topColor.r * (1.0f - localT) + (float)atm->sky.horizonColor.r * localT);
            color.g = (unsigned char)((float)atm->sky.topColor.g * (1.0f - localT) + (float)atm->sky.horizonColor.g * localT);
            color.b = (unsigned char)((float)atm->sky.topColor.b * (1.0f - localT) + (float)atm->sky.horizonColor.b * localT);
            color.a = 255;
        } else {
            // Segunda metade: horizonColor -> bottomColor
            float localT = (t - 0.5f) * 2.0f; // 0.0 a 1.0
            color.r = (unsigned char)((float)atm->sky.horizonColor.r * (1.0f - localT) + (float)atm->sky.bottomColor.r * localT);
            color.g = (unsigned char)((float)atm->sky.horizonColor.g * (1.0f - localT) + (float)atm->sky.bottomColor.g * localT);
            color.b = (unsigned char)((float)atm->sky.horizonColor.b * (1.0f - localT) + (float)atm->sky.bottomColor.b * localT);
            color.a = 255;
        }
        
        // Calcula posição Y da faixa
        float y1 = (float)screenHeight * t;
        float y2 = (float)screenHeight * ((float)(i + 1) / (float)gradientSteps);
        
        // Desenha retângulo horizontal
        DrawRectangle(0, (int)y1, screenWidth, (int)(y2 - y1) + 1, color);
    }
}

Color Atmosphere_GetFogColor(Atmosphere* atm) {
    if (!atm || !atm->initialized) {
        return (Color){20, 20, 30, 255}; // Cor padrão
    }
    return atm->fog.fogColor;
}
