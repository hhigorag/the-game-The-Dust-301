#ifndef LIGHTING_H
#define LIGHTING_H

#include <raylib.h>
#include <stdbool.h>

// ============================================================================
// LIGHTING - Iluminação Direcional Fake
// ============================================================================
// Sistema de iluminação fake (apenas visual, sem cálculo real de luz).
// Luz direcional simulada via cálculo de normal das faces e direção da luz.
// ============================================================================

typedef struct {
    Vector3 direction;         // Direção da luz (normalizada, geralmente do sol)
    Color lightColor;           // Cor da luz (branco/amarelo claro)
    Color ambientColor;         // Cor ambiente (sombra mínima)
    float intensity;            // Intensidade da luz (0.0 a 1.0)
    float ambientIntensity;     // Intensidade ambiente (0.0 a 1.0)
    bool enabled;
} DirectionalLight;

typedef struct {
    DirectionalLight sunLight;
    bool initialized;
} LightingSystem;

// Inicializa sistema de iluminação com valores padrão
void Lighting_Init(LightingSystem* lighting);

// Limpa recursos do sistema de iluminação
void Lighting_Shutdown(LightingSystem* lighting);

// Configura luz direcional (sol)
void Lighting_SetDirectionalLight(LightingSystem* lighting, 
                                   Vector3 direction, Color lightColor, 
                                   Color ambientColor, float intensity, float ambientIntensity);

// Aplica iluminação fake a uma cor baseada na normal da face
// Retorna a cor com iluminação aplicada
// normal: vetor normal da face (normalizado)
// baseColor: cor base do material
Color Lighting_ApplyDirectionalLight(LightingSystem* lighting, 
                                     Vector3 normal, Color baseColor);

// Obtém a direção da luz atual
Vector3 Lighting_GetLightDirection(LightingSystem* lighting);

// Habilita/desabilita iluminação
void Lighting_SetEnabled(LightingSystem* lighting, bool enabled);

#endif // LIGHTING_H
