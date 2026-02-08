#ifndef ARC_TERMINAL_ARC_RENDER_H
#define ARC_TERMINAL_ARC_RENDER_H

#include <raylib.h>
#include "arc_common.h"

void Arc_DrawShellText(Font font, const char* text, float x, float y, float size, Color color);
void Arc_DrawShellTextTypewriter(Font font, const char* text, float x, float y, float size, Color color,
                                 float timer, float duration);
void Arc_DrawSemiDynamicText(Font font, const char* label, const char* value, float x, float y, float size, Color color,
                             float typewriterTimer, float typewriterDuration, int isUpdating);

Color Arc_GetRiskColor(int riskLevel);
Color Arc_GetAnomalyColor(int anomalyIndex);

#endif
