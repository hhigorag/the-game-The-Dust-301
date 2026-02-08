#include "app/ui/arc_terminal/arc_render.h"
#include <string.h>
#include <stdio.h>

void Arc_DrawShellText(Font font, const char* text, float x, float y, float size, Color color) {
    if (!text) return;
    Vector2 pos1 = { x, y };
    Vector2 pos2 = { x + 1.0f, y };
    DrawTextEx(font, text, pos2, size, 2, color);
    DrawTextEx(font, text, pos1, size, 2, color);
}

void Arc_DrawShellTextTypewriter(Font font, const char* text, float x, float y, float size, Color color, float timer, float duration) {
    if (!text) return;
    int textLen = (int)strlen(text);
    if (textLen == 0) return;
    int charsToShow = textLen;
    if (timer < duration && duration > 0.0f) {
        float progress = timer / duration;
        charsToShow = (int)(progress * textLen);
        if (charsToShow > textLen) charsToShow = textLen;
        if (charsToShow < 0) charsToShow = 0;
    }
    char displayText[512];
    strncpy(displayText, text, charsToShow);
    displayText[charsToShow] = '\0';
    Arc_DrawShellText(font, displayText, x, y, size, color);
}

void Arc_DrawSemiDynamicText(Font font, const char* label, const char* value, float x, float y, float size, Color color,
                             float typewriterTimer, float typewriterDuration, int isUpdating) {
    if (isUpdating) {
        float dotsTimer = (float)GetTime();
        int dotsCount = (int)(dotsTimer * 4.0f) % 4;
        char dots[4] = "";
        for (int i = 0; i < dotsCount && i < 3; i++) strcat(dots, ".");
        char labelValueText[256];
        snprintf(labelValueText, sizeof(labelValueText), "%s: %s", label, value);
        Arc_DrawShellTextTypewriter(font, labelValueText, x, y, size, color, typewriterTimer, typewriterDuration);
        Vector2 labelValueSize = MeasureTextEx(font, labelValueText, size, 1);
        float marginLeft = 25.0f;
        char updatingText[32];
        snprintf(updatingText, sizeof(updatingText), "UPDATING%s", dots);
        Arc_DrawShellText(font, updatingText, x + labelValueSize.x + marginLeft, y, size, WHITE);
    } else {
        char displayText[256];
        snprintf(displayText, sizeof(displayText), "%s: %s", label, value);
        Arc_DrawShellTextTypewriter(font, displayText, x, y, size, color, typewriterTimer, typewriterDuration);
    }
}

Color Arc_GetRiskColor(int riskLevel) {
    switch (riskLevel) {
        case 1: return ARC_COLOR_GREEN;
        case 2: return (Color){255, 255, 0, 255};
        case 3: return (Color){255, 0, 0, 255};
        case 4: return (Color){128, 0, 128, 255};
        default: return ARC_COLOR_GREEN;
    }
}

Color Arc_GetAnomalyColor(int anomalyIndex) {
    switch (anomalyIndex) {
        case 0: return ARC_COLOR_GREEN;
        case 1: return ARC_COLOR_GREEN;
        case 2: return (Color){255, 255, 0, 255};
        case 3: return (Color){255, 255, 255, 255};
        case 4: return (Color){255, 80, 80, 255};
        case 5: return (Color){180, 0, 255, 255};
        case 6: return (Color){255, 255, 255, 255};
        case 7: return (Color){0, 255, 255, 255};
        default: return ARC_COLOR_GREEN;
    }
}
