#include "app/ui/arc_clim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CLIMATE_DISABLED 0

static ClimateData currentClimate = {0};
static ClimateVisualState visualState = {0};
static int lastUpdateHour = -1, lastUpdateMinute = -1, lastUpdateSecond = -1;
static int currentPlanetIndex = 0;

static float weatherUpdateInterval = 90.0f, riskLevelUpdateInterval = 120.0f;
static float statusUpdateInterval = 90.0f, temperatureUpdateInterval = 90.0f;
static float criticalUpdateInterval = 90.0f;

typedef struct {
    float baseTemp, tempVariation, gravity;
    const char* envType, *atmosphere;
    int baseRiskLevel;
    const char* typicalWeather, *typicalResources;
} PlanetClimateBase;

static const PlanetClimateBase planetClimates[] = {
    {-39.0f, 25.0f, 1.1f, "TERRESTRIAL", "THIN", 2, "UNSTABLE", "MINERALS DETECTED"},
    {-5.0f, 20.0f, 0.93f, "OCEANIC", "BREATHABLE", 1, "STABLE", "ENERGY SOURCES FOUND"},
    {15.0f, 15.0f, 1.4f, "TERRESTRIAL", "DENSE", 2, "CLEAR", "MINERALS DETECTED"},
    {5.0f, 18.0f, 1.3f, "TERRESTRIAL", "TOXIC", 2, "FOGGY", "SCANNING..."},
    {25.0f, 20.0f, 1.2f, "DESERT", "THIN", 3, "STORM", "NO RESOURCES"}
};

static const char* statusOptions[] = {"SCANNING", "STABLE", "UNSTABLE", "CRITICAL", "UNDEFINED"};
static const char* envTypeOptions[] = {"TERRESTRIAL", "GAS GIANT", "ICE WORLD", "DESERT", "OCEANIC", "N/A"};
static const char* loadOptions[] = {"STANDARD", "LIGHT", "HEAVY", "EXTREME"};
static const char* resourceOptions[] = {"SCANNING...", "MINERALS DETECTED", "ENERGY SOURCES FOUND", "NO RESOURCES", "VOLATILE DETECTED", "UNSTABLE SOURCE", "SCANNING..."};
static const char* anomalyOptions[] = {"NONE", "DETECTED", "MULTIPLE SIGNALS", "SOURCE UNKNOWN", "HIGH ACTIVITY", "EXTREME", "SIGNAL LOST", "BEWARE THE DUST"};
static const char* notesOptions[] = {"STABLE ENVIRONMENT", "MODERATE CONDITIONS", "HIGH RISK - EXERCISE CAUTION", "UNSTABLE READINGS", "SYSTEM RECALIBRATING", "ANOMALIES DETECTED", "CAUTION ADVISED", "ALL SYSTEMS NOMINAL"};
static const char* weatherOptions[] = {"STABLE", "UNSTABLE", "STORM", "CLEAR", "FOGGY", "ELECTRICAL STORM", "WIND PATTERNS"};
static const char* atmosphereOptions[] = {"BREATHABLE", "TOXIC", "THIN", "DENSE", "NONE"};

static float GetTimeBasedValueWithSecond(int hour, int minute, int second, float baseValue, float variation) {
    int seed = hour * 3600 + minute * 60 + second;
    srand(seed);
    float randomFactor = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
    return baseValue + (randomFactor * variation);
}

static void UpdateTemperature(int hour, int minute, int second, int planetIndex) {
    if (planetIndex < 0 || planetIndex >= 5) planetIndex = 0;
    const PlanetClimateBase* planet = &planetClimates[planetIndex];
    float timeOfDay = hour + minute / 60.0f + second / 3600.0f;
    float baseTemp = planet->baseTemp + (sinf(timeOfDay * 3.14159f / 12.0f) * (planet->tempVariation / 2.0f));
    float targetTemp = GetTimeBasedValueWithSecond(hour, minute, 0, baseTemp, planet->tempVariation / 20.0f);
    if (lastUpdateHour == -1) {
        currentClimate.temperature = targetTemp;
    } else {
        float diff = targetTemp - currentClimate.temperature;
        float moveSpeed = (fabsf(diff) > 3.0f) ? 2.0f : 1.0f;
        if (diff > 0) { currentClimate.temperature += moveSpeed; if (currentClimate.temperature > targetTemp) currentClimate.temperature = targetTemp; }
        else if (diff < 0) { currentClimate.temperature -= moveSpeed; if (currentClimate.temperature < targetTemp) currentClimate.temperature = targetTemp; }
        float variation = GetTimeBasedValueWithSecond(hour, minute, 0, 0.0f, 0.2f);
        currentClimate.temperature += variation;
        float maxTemp = planet->baseTemp + planet->tempVariation, minTemp = planet->baseTemp - planet->tempVariation;
        if (currentClimate.temperature > maxTemp) currentClimate.temperature = maxTemp;
        if (currentClimate.temperature < minTemp) currentClimate.temperature = minTemp;
    }
}

static void UpdateSemiDynamicProperties(int hour, int minute, int second, int planetIndex) {
    (void)second;
    if (planetIndex < 0 || planetIndex >= 5) planetIndex = 0;
    const PlanetClimateBase* planet = &planetClimates[planetIndex];
    int seed = hour * 60 + minute;
    srand(seed);
    if (visualState.weatherUpdateTimer >= weatherUpdateInterval) {
        visualState.weatherUpdating = 1;
        visualState.weatherUpdateTimer = 0.0f;
        int weatherIndex = 0;
        for (int i = 0; i < 7; i++) { if (strcmp(weatherOptions[i], planet->typicalWeather) == 0) { weatherIndex = i; break; } }
        weatherIndex = (weatherIndex + (hour + minute) % 5) % 7;
        currentClimate.weather = weatherOptions[weatherIndex];
    }
    if (visualState.riskLevelUpdateTimer >= riskLevelUpdateInterval) {
        visualState.riskLevelUpdating = 1;
        visualState.riskLevelUpdateTimer = 0.0f;
        int tempRisk = (currentClimate.temperature < -30.0f) ? 2 : (currentClimate.temperature < -10.0f || currentClimate.temperature > 30.0f) ? 1 : 0;
        currentClimate.riskLevel = planet->baseRiskLevel + tempRisk;
        if (currentClimate.riskLevel > 4) currentClimate.riskLevel = 4;
        if (currentClimate.riskLevel < 1) currentClimate.riskLevel = 1;
    }
    if (visualState.statusUpdateTimer >= statusUpdateInterval) {
        visualState.statusUpdating = 1;
        visualState.statusUpdateTimer = 0.0f;
        int statusIndex = (hour < 6 || hour > 22) ? 2 : (rand() % 3);
        if (planetIndex == 0 && rand() % 2 == 0) statusIndex = 2;
        currentClimate.status = statusOptions[statusIndex];
    }
    if (visualState.temperatureUpdateTimer >= temperatureUpdateInterval) {
        visualState.temperatureUpdating = 1;
        visualState.temperatureUpdateTimer = 0.0f;
    }
}

static void UpdateCriticalProperties(int hour, int minute, int second, int planetIndex) {
    (void)second;
    if (planetIndex < 0 || planetIndex >= 5) planetIndex = 0;
    int seed = hour * 60 + minute;
    srand(seed);
    if (visualState.anomaliesBlinkTimer >= criticalUpdateInterval) {
        visualState.anomaliesBlinking = 1;
        visualState.anomaliesBlinkTimer = 0.0f;
        visualState.notesBlinking = 1;
        visualState.notesBlinkTimer = 0.0f;
        int anomalyIndex = ((rand() % 100) < 8) ? 7 : (hour * 60 + minute) % 7;
        currentClimate.anomalyIndex = anomalyIndex;
        currentClimate.anomalies = anomalyOptions[anomalyIndex];
        currentClimate.notes = notesOptions[anomalyIndex];
    }
    if (visualState.resourcesBlinkTimer >= criticalUpdateInterval) {
        visualState.resourcesBlinking = 1;
        visualState.resourcesBlinkTimer = 0.0f;
        if (planetIndex == 0 || planetIndex == 4) {
            int resourceIndex = (hour * 60 + minute) % 7;
            currentClimate.resources = resourceOptions[resourceIndex];
        }
    }
}

void InitClimate(void) {
    memset(&currentClimate, 0, sizeof(currentClimate));
    currentClimate.temperature = -45.0f;
    currentClimate.riskLevel = 2;
    currentClimate.envType = envTypeOptions[0];
    currentClimate.status = statusOptions[0];
    currentClimate.recommendedLoad = loadOptions[0];
    currentClimate.anomalyIndex = 0;
    currentClimate.notes = notesOptions[0];
    currentClimate.resources = resourceOptions[0];
    currentClimate.anomalies = anomalyOptions[0];
    currentClimate.weather = weatherOptions[0];
    currentClimate.gravity = 1.2f;
    currentClimate.atmosphere = atmosphereOptions[1];
    memset(&visualState, 0, sizeof(visualState));
    lastUpdateHour = lastUpdateMinute = lastUpdateSecond = -1;
}

void UpdateClimate(int hour, int minute, int second, int planetIndex, float deltaTime) {
    if (CLIMATE_DISABLED) return;
    if (planetIndex < 0 || planetIndex >= 5) planetIndex = 0;
    if (currentPlanetIndex != planetIndex) {
        lastUpdateHour = lastUpdateMinute = lastUpdateSecond = -1;
        currentPlanetIndex = planetIndex;
        memset(&visualState, 0, sizeof(visualState));
        const PlanetClimateBase* planet = &planetClimates[planetIndex];
        currentClimate.envType = planet->envType;
        currentClimate.gravity = planet->gravity;
        currentClimate.atmosphere = planet->atmosphere;
        currentClimate.anomalyIndex = 0;
        currentClimate.anomalies = anomalyOptions[0];
        currentClimate.notes = notesOptions[0];
    }
    visualState.weatherUpdateTimer += deltaTime;
    visualState.riskLevelUpdateTimer += deltaTime;
    visualState.statusUpdateTimer += deltaTime;
    visualState.temperatureUpdateTimer += deltaTime;
    visualState.anomaliesBlinkTimer += deltaTime;
    visualState.notesBlinkTimer += deltaTime;
    visualState.resourcesBlinkTimer += deltaTime;
    static float temperatureCycleAccum = 0.0f;
    int shouldUpdate = (lastUpdateHour == -1);
    if (!shouldUpdate) {
        temperatureCycleAccum += deltaTime;
        if (temperatureCycleAccum >= 90.0f) { temperatureCycleAccum = 0.0f; shouldUpdate = 1; }
    } else temperatureCycleAccum = 0.0f;
    if (shouldUpdate) { UpdateTemperature(hour, minute, 0, planetIndex); lastUpdateSecond = second; }
    UpdateSemiDynamicProperties(hour, minute, second, planetIndex);
    UpdateCriticalProperties(hour, minute, second, planetIndex);
    static float weatherUpdatingTimer = 0, riskLevelUpdatingTimer = 0, statusUpdatingTimer = 0, temperatureUpdatingTimer = 0;
    static float anomaliesBlinkingTimer = 0, notesBlinkingTimer = 0, resourcesBlinkingTimer = 0;
    if (visualState.weatherUpdating) { weatherUpdatingTimer += deltaTime; if (weatherUpdatingTimer >= 1.5f) { visualState.weatherUpdating = 0; weatherUpdatingTimer = 0; } }
    if (visualState.riskLevelUpdating) { riskLevelUpdatingTimer += deltaTime; if (riskLevelUpdatingTimer >= 1.5f) { visualState.riskLevelUpdating = 0; riskLevelUpdatingTimer = 0; } }
    if (visualState.statusUpdating) { statusUpdatingTimer += deltaTime; if (statusUpdatingTimer >= 1.5f) { visualState.statusUpdating = 0; statusUpdatingTimer = 0; } }
    if (visualState.temperatureUpdating) { temperatureUpdatingTimer += deltaTime; if (temperatureUpdatingTimer >= 1.5f) { visualState.temperatureUpdating = 0; temperatureUpdatingTimer = 0; } }
    if (visualState.anomaliesBlinking) { anomaliesBlinkingTimer += deltaTime; if (anomaliesBlinkingTimer >= 0.5f) { visualState.anomaliesBlinking = 0; anomaliesBlinkingTimer = 0; } }
    if (visualState.notesBlinking) { notesBlinkingTimer += deltaTime; if (notesBlinkingTimer >= 0.5f) { visualState.notesBlinking = 0; notesBlinkingTimer = 0; } }
    if (visualState.resourcesBlinking) { resourcesBlinkingTimer += deltaTime; if (resourcesBlinkingTimer >= 0.5f) { visualState.resourcesBlinking = 0; resourcesBlinkingTimer = 0; } }
    if (currentClimate.riskLevel >= 3) currentClimate.recommendedLoad = loadOptions[2];
    else if (currentClimate.riskLevel == 2) currentClimate.recommendedLoad = loadOptions[0];
    else currentClimate.recommendedLoad = loadOptions[1];
    lastUpdateHour = hour;
    lastUpdateMinute = minute;
}

ClimateData GetClimateData(int planetIndex) {
    if (planetIndex < 0 || planetIndex >= 5) planetIndex = 0;
    if (CLIMATE_DISABLED) {
        ClimateData d = {0};
        const PlanetClimateBase* p = &planetClimates[planetIndex];
        d.temperature = p->baseTemp; d.riskLevel = p->baseRiskLevel; d.envType = p->envType;
        d.atmosphere = p->atmosphere; d.gravity = p->gravity; d.status = statusOptions[0];
        d.recommendedLoad = loadOptions[0]; d.anomalyIndex = 0; d.notes = notesOptions[0];
        d.resources = p->typicalResources; d.anomalies = anomalyOptions[0]; d.weather = p->typicalWeather;
        return d;
    }
    if (currentPlanetIndex != planetIndex) {
        currentPlanetIndex = planetIndex;
        const PlanetClimateBase* planet = &planetClimates[planetIndex];
        currentClimate.envType = planet->envType;
        currentClimate.gravity = planet->gravity;
        currentClimate.atmosphere = planet->atmosphere;
    }
    return currentClimate;
}

ClimateVisualState GetClimateVisualState(int planetIndex) {
    (void)planetIndex;
    if (CLIMATE_DISABLED) return (ClimateVisualState){0};
    if (planetIndex < 0 || planetIndex >= 5) planetIndex = 0;
    return visualState;
}
