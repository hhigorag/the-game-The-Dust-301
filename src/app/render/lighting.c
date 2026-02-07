#include "app/render/lighting.h"
#include <math.h>

// ============================================================================
// LIGHTING - Implementação
// ============================================================================

static float Vec3_Dot(Vector3 a, Vector3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static float Vec3_Length(Vector3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static Vector3 Vec3_Normalize(Vector3 v) {
    float len = Vec3_Length(v);
    if (len > 0.0001f) {
        return (Vector3){v.x / len, v.y / len, v.z / len};
    }
    return (Vector3){0.0f, 0.0f, 0.0f};
}

void Lighting_Init(LightingSystem* lighting) {
    if (!lighting) return;
    
    // Luz direcional padrão: sol vindo do alto e ligeiramente de frente
    lighting->sunLight.direction = (Vector3){0.3f, -0.8f, 0.5f}; // Normalizado depois
    lighting->sunLight.direction = Vec3_Normalize(lighting->sunLight.direction);
    
    lighting->sunLight.lightColor = (Color){255, 250, 240, 255}; // Branco quente/amarelo claro
    lighting->sunLight.ambientColor = (Color){50, 50, 60, 255};  // Azul escuro ambiente
    lighting->sunLight.intensity = 0.8f;                         // 80% de intensidade
    lighting->sunLight.ambientIntensity = 0.3f;                   // 30% de ambiente
    lighting->sunLight.enabled = true;
    
    lighting->initialized = true;
}

void Lighting_Shutdown(LightingSystem* lighting) {
    if (!lighting) return;
    lighting->initialized = false;
}

void Lighting_SetDirectionalLight(LightingSystem* lighting, 
                                   Vector3 direction, Color lightColor, 
                                   Color ambientColor, float intensity, float ambientIntensity) {
    if (!lighting || !lighting->initialized) return;
    
    lighting->sunLight.direction = Vec3_Normalize(direction);
    lighting->sunLight.lightColor = lightColor;
    lighting->sunLight.ambientColor = ambientColor;
    lighting->sunLight.intensity = intensity;
    lighting->sunLight.ambientIntensity = ambientIntensity;
}

Color Lighting_ApplyDirectionalLight(LightingSystem* lighting,
                                     Vector3 normal, Color baseColor) {
    if (!lighting || !lighting->initialized || !lighting->sunLight.enabled) {
        return baseColor;
    }

    Vector3 faceNormal = Vec3_Normalize(normal);
    /* Wrap: ndl = dot(n,l)*0.5+0.5, clamp; factor = 0.6+0.4*ndl; color *= factor (uma vez). */
    float ndl = Vec3_Dot(faceNormal, lighting->sunLight.direction) * 0.5f + 0.5f;
    if (ndl < 0.0f) ndl = 0.0f;
    if (ndl > 1.0f) ndl = 1.0f;
    ndl = ndl * ndl; /* curva leve: mais contraste sem specular */
    float factor = 0.6f + 0.4f * ndl;

    /* Ambient hemisférico: mais azul em cima, mais escuro em baixo */
    float hemi = faceNormal.y * 0.5f + 0.5f;
    if (hemi < 0.0f) hemi = 0.0f;
    if (hemi > 1.0f) hemi = 1.0f;
    float hemiFactor = 0.5f + 0.5f * hemi;
    factor *= hemiFactor;

    float r = (float)baseColor.r * factor;
    float g = (float)baseColor.g * factor;
    float b = (float)baseColor.b * factor;
    if (r > 255.0f) r = 255.0f;
    if (g > 255.0f) g = 255.0f;
    if (b > 255.0f) b = 255.0f;
    if (r < 0.0f) r = 0.0f;
    if (g < 0.0f) g = 0.0f;
    if (b < 0.0f) b = 0.0f;

    Color result;
    result.r = (unsigned char)r;
    result.g = (unsigned char)g;
    result.b = (unsigned char)b;
    result.a = baseColor.a;
    return result;
}

Vector3 Lighting_GetLightDirection(LightingSystem* lighting) {
    if (!lighting || !lighting->initialized) {
        return (Vector3){0.0f, -1.0f, 0.0f}; // Direção padrão (de cima)
    }
    return lighting->sunLight.direction;
}

void Lighting_SetEnabled(LightingSystem* lighting, bool enabled) {
    if (!lighting || !lighting->initialized) return;
    lighting->sunLight.enabled = enabled;
}
