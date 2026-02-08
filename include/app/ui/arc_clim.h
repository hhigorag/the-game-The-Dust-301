#ifndef ARC_CLIM_H
#define ARC_CLIM_H

typedef struct {
    float temperature;
    int riskLevel;
    const char* envType;
    const char* status;
    const char* recommendedLoad;
    const char* notes;
    const char* resources;
    const char* anomalies;
    int anomalyIndex;
    const char* weather;
    float gravity;
    const char* atmosphere;
} ClimateData;

typedef struct {
    int weatherUpdating, riskLevelUpdating, statusUpdating, temperatureUpdating;
    int anomaliesBlinking, notesBlinking, resourcesBlinking;
    float weatherUpdateTimer, riskLevelUpdateTimer, statusUpdateTimer, temperatureUpdateTimer;
    float anomaliesBlinkTimer, notesBlinkTimer, resourcesBlinkTimer;
} ClimateVisualState;

void InitClimate(void);
void UpdateClimate(int hour, int minute, int second, int planetIndex, float deltaTime);
ClimateData GetClimateData(int planetIndex);
ClimateVisualState GetClimateVisualState(int planetIndex);

#endif
