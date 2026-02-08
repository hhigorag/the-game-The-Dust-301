/*
 * Shell render: desenho completo do PRIORITY_MANAGE, NAVIGATION, ARC_REPORT.
 * Reimportado de terminal-with-raylib, adaptado para 1600x920.
 */
#include "app/ui/arc_terminal/shell.h"
#include "app/ui/arc_terminal/shell_private.h"
#include "app/ui/arc_terminal/arc_common.h"
#include "app/ui/arc_terminal/arc_render.h"
#include "app/ui/arc_terminal/arc_utils.h"
#include "app/ui/arc_clim.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <raylib.h>

static const char* planetNames[] = {
    "PROXIMA CENTAURI B", "TRAPPIST-1E", "KEPLER-442B", "GLIESE 667CC", "WOLF 1061C"
};
static const int planetCount = 5;
static const char* missionNames[] = { "Collection 8b-071", "Collection 8b-099", "Collection 8c-173", "Collection 7d-991" };
static const int missionCount = 4;
static const char* landingStepNames[] = {"ENTER ORBIT","DEPLOY SCANNERS","GENERATE MAP","INITIATE LANDING","OPEN AIRLOCK"};

static int get_landing_bracket(const ArcShellContext* s, int step) {
    if (step == 0) return s->landingDone[0] ? 1 : 0;
    if (step == 1) return s->landingDone[1] ? 1 : (s->landingDone[0] ? 0 : -1);
    if (step == 2) return s->landingDone[2] ? 1 : (s->landingDone[1] ? 0 : -1);
    if (step == 3) return s->landingDone[3] ? 1 : (s->landingDone[2] ? 0 : -1);
    if (step == 4) return s->landingDone[4] ? 1 : (s->landingDone[3] ? 0 : -1);
    return -1;
}

void Arc_Shell_Render(ArcShellContext* s, Font font, Texture2D logo, bool logoOk,
    const char* username, const char* cpuName, unsigned long long totalRam,
    float screenW, float screenH) {
    if (!s) return;

    float dividerY = screenH * (ARC_DIVIDER_Y_OFFSET / 1080.0f);
    float menuDividerX = screenW - 375.0f;
    float wikiDividerX = 40.0f + 300.0f;
    float lineSpacing = 30.0f;

    if (logoOk && logo.id != 0) {
        float wm = 0.8f;
        float ww = (float)logo.width * wm, wh = (float)logo.height * wm;
        DrawTextureEx(logo, (Vector2){(screenW - ww) / 2, (screenH - wh) / 2}, 0, wm, (Color){255, 255, 255, 30});
    }

    char dateTimeStr[32];
    Arc_GetFormattedDateTime(dateTimeStr, sizeof(dateTimeStr));
    Arc_DrawShellText(font, dateTimeStr, 40, 50, 16, WHITE);

    DrawRectangleLinesEx((Rectangle){20, 20, screenW - 40, screenH - 40}, 2, ARC_COLOR_GREEN);
    const char* header = Arc_GetModeTitle(s->currentMode);
    Vector2 headerSize = MeasureTextEx(font, header, 25, 2);
    Arc_DrawShellText(font, header, (screenW - headerSize.x) / 2, 35, 25, ARC_COLOR_GREEN);

    if (s->currentMode == ARC_MODE_PRIORITY_MANAGE) {
        DrawLineEx((Vector2){40, 75}, (Vector2){screenW - 40, 75}, 1, ARC_COLOR_GREEN);
        float spacing = 200.0f;
        float totalMenuW = (s->menuCount - 1) * spacing;
        float startX = (screenW - totalMenuW) / 2;
        for (int i = 0; i < s->menuCount; i++) {
            float cx = startX + i * spacing;
            Vector2 tSize = MeasureTextEx(font, s->menuItems[i], 20, 1);
            float itemX = cx - tSize.x / 2;
            if (i == s->menuSelected)
                DrawRectangleRec((Rectangle){itemX - 8, 88, tSize.x + 16, 25}, (Color){0, 255, 0, 40});
            Color c = s->menuEnabled[i] ? ARC_COLOR_GREEN : WHITE;
            Arc_DrawShellText(font, s->menuItems[i], itemX, 90, 20, c);
            if (i < s->menuCount - 1)
                Arc_DrawShellText(font, "|", cx + spacing/2 - 5, 90, 20, ARC_COLOR_GREEN);
        }
        DrawLineEx((Vector2){40, 115}, (Vector2){screenW - 40, 115}, 1, ARC_COLOR_GREEN);

        if (!s->menuEnabled[s->menuSelected]) {
            Vector2 msgSize = MeasureTextEx(font, "MODULE NOT AVAILABLE", 25, 1);
            Arc_DrawShellText(font, "MODULE NOT AVAILABLE", (screenW - msgSize.x)/2, screenH/2, 25, WHITE);
        } else if (s->menuSelected == s->menuCount - 1) {
            float scanSepX = startX + 2 * spacing + spacing/2 - 5;
            float centerX = scanSepX + 30;
            DrawLineEx((Vector2){centerX, 115}, (Vector2){centerX, screenH - 75}, 1, ARC_COLOR_GREEN);
            Arc_DrawShellText(font, "COMMANDS", 60, 135, 18, (Color){120, 255, 120, 255});
            float cmdStartY = 160;
            int maxLeft = (int)((screenH - cmdStartY - 120) / 25);
            int leftStart = (s->lineCount > maxLeft) ? s->lineCount - maxLeft : 0;
            float maxTextW = centerX - 60;
            int row = 0;
            for (int i = leftStart; i < s->lineCount && row < maxLeft; i++) {
                const char* txt = s->terminalHistory[i];
                if (!txt || !txt[0]) continue;
                Vector2 ts = MeasureTextEx(font, txt, 20, 2);
                if (ts.x <= maxTextW) {
                    Arc_DrawShellText(font, txt, 40, cmdStartY + row * 25, 20, ARC_COLOR_GREEN);
                    row++;
                } else {
                    char buf[256];
                    size_t len = strlen(txt);
                    size_t pos = 0;
                    while (pos < len && row < maxLeft) {
                        size_t br = pos;
                        for (size_t j = pos; j < len && j < pos + 80; j++) {
                            if ((unsigned char)txt[j] <= 32) br = j + 1;
                            char tmp[256];
                            size_t tl = j - pos + 1;
                            if (tl >= sizeof(tmp)) break;
                            memcpy(tmp, txt + pos, tl);
                            tmp[tl] = '\0';
                            if (MeasureTextEx(font, tmp, 20, 2).x > maxTextW) break;
                            br = j + 1;
                        }
                        size_t ln = br - pos;
                        if (ln >= sizeof(buf)) ln = sizeof(buf) - 1;
                        memcpy(buf, txt + pos, ln);
                        buf[ln] = '\0';
                        Arc_DrawShellText(font, buf, 40, cmdStartY + row * 25, 20, ARC_COLOR_GREEN);
                        row++;
                        pos = br;
                        while (pos < len && (unsigned char)txt[pos] <= 32) pos++;
                    }
                }
            }
            Arc_DrawShellText(font, "EVENT LOG  (PGUP/PGDN)", centerX + 20, 135, 18, (Color){120, 255, 120, 255});
            int visible = (int)((screenH - 160 - 120) / 22);
            if (visible < 1) visible = 1;
            if (visible > 30) visible = 30;
            int start = s->logCount - visible - s->logScroll;
            if (start < 0) start = 0;
            int end = start + visible;
            if (end > s->logCount) end = s->logCount;
            for (int i = start, r = 0; i < end; i++, r++)
                Arc_DrawShellText(font, s->logHistory[i], centerX + 20, 160 + r * 22, 18, ARC_COLOR_GREEN);
        }
    } else if (s->currentMode == ARC_MODE_NAVIGATION) {
        if (s->openAirlockSwitchActive) {
            DrawRectangle(0, 0, (int)screenW, (int)screenH, BLACK);
            int dc = (int)((float)GetTime() * 4) % 4;
            char adots[5] = {0};
            for (int i = 0; i < dc && i < 3; i++) adots[i] = '.';
            adots[dc < 3 ? dc : 3] = '\0';
            char msg[64];
            snprintf(msg, sizeof(msg), "AUTOMATIC SWITCH TO MANAGE SYSTEM%s", adots);
            Vector2 ms = MeasureTextEx(font, msg, 24, 1);
            Arc_DrawShellText(font, msg, (screenW - ms.x)/2, screenH/2 - 20, 24, ARC_COLOR_GREEN);
        } else {
            DrawLineEx((Vector2){40, 75}, (Vector2){screenW - 40, 75}, 1, ARC_COLOR_GREEN);
            DrawLineEx((Vector2){40, 115}, (Vector2){screenW - 40, 115}, 1, ARC_COLOR_GREEN);
            DrawLineEx((Vector2){menuDividerX, 115}, (Vector2){menuDividerX, screenH - 75}, 1, ARC_COLOR_GREEN);
            DrawLineEx((Vector2){wikiDividerX, 115}, (Vector2){wikiDividerX, screenH - 75}, 1, ARC_COLOR_GREEN);

            BeginScissorMode((int)40, 115, (int)(wikiDividerX - 40), (int)(screenH - 190));
            float leftY = 140;
            if (s->navSelected == 0) {
                Arc_DrawShellText(font, "CATALOGED PLANETS", 60, leftY, 25, ARC_COLOR_GREEN);
                Arc_DrawShellText(font, "SELECT A PLANET:", 60, leftY + lineSpacing, 18, ARC_COLOR_GREEN);
                float listY = leftY + lineSpacing * 2.5f;
                for (int i = 0; i < planetCount; i++) {
                    const char* pre = (i == s->planetSelected) ? ">" : " ";
                    char line[128];
                    snprintf(line, sizeof(line), "%s %s", pre, planetNames[i]);
                    Arc_DrawShellText(font, line, 60, listY + i * lineSpacing, 20, ARC_COLOR_GREEN);
                }
            } else {
                Arc_DrawShellText(font, "MISSIONS AVAILABLE", 60, leftY, 25, ARC_COLOR_GREEN);
                Arc_DrawShellText(font, "SELECT MISSION:", 60, leftY + lineSpacing, 18, ARC_COLOR_GREEN);
                float listY = leftY + lineSpacing * 2.5f;
                for (int i = 0; i < missionCount; i++) {
                    const char* pre = (i == s->missionSelected) ? ">" : " ";
                    char line[128];
                    snprintf(line, sizeof(line), "%s %s", pre, missionNames[i]);
                    Arc_DrawShellText(font, line, 60, listY + i * lineSpacing, 20, ARC_COLOR_GREEN);
                }
            }
            EndScissorMode();

            float planetWikiH = dividerY - 115;
            BeginScissorMode((int)wikiDividerX, 115, (int)(menuDividerX - wikiDividerX), (int)planetWikiH);
            float rightY = 140 - s->wikiScroll;
            if (s->navSelected == 0) {
                ClimateData clim = GetClimateData(s->planetSelected);
                ClimateVisualState vis = GetClimateVisualState(s->planetSelected);
                int wikiNoInfo = (s->landingDone[0] && !s->landingDone[1]);
                int wikiLoad = (s->landingLoadingStep == 1 && (float)GetTime() - s->deployScannersWikiStart < 3.0f);

                char title[128];
                snprintf(title, sizeof(title), "PLANET: %s", planetNames[s->planetSelected]);
                Arc_DrawShellTextTypewriter(font, title, wikiDividerX + 20, rightY, 30, ARC_COLOR_GREEN, s->typewriterTimer, s->typewriterDuration);
                float infoY = rightY + 50;
                if (wikiLoad) {
                    int dc = (int)((float)GetTime() * 4) % 4;
                    char sd[8] = "[";
                    for (int i = 0; i < dc && i < 3; i++) sd[1+i] = '.';
                    sd[1+dc] = ']'; sd[2+dc] = '\0';
                    Arc_DrawShellText(font, "STATUS: ", wikiDividerX + 20, infoY, 20, ARC_COLOR_GREEN);
                    Arc_DrawShellText(font, sd, wikiDividerX + 20 + MeasureTextEx(font, "STATUS: ", 20, 1).x, infoY, 20, ARC_COLOR_GREEN);
                } else if (wikiNoInfo) {
                    Arc_DrawShellText(font, "STATUS: [ NO INFO ]", wikiDividerX + 20, infoY, 20, ARC_COLOR_GREEN);
                } else {
                    Arc_DrawSemiDynamicText(font, "STATUS", clim.status, wikiDividerX + 20, infoY, 20, ARC_COLOR_GREEN, s->typewriterTimer, s->typewriterDuration, vis.statusUpdating);
                }
                char riskStr[32], tempStr[32], gravStr[32];
                snprintf(riskStr, sizeof(riskStr), "%d", clim.riskLevel);
                snprintf(tempStr, sizeof(tempStr), "%.0fC", clim.temperature);
                snprintf(gravStr, sizeof(gravStr), "%.1fG", clim.gravity);
                Arc_DrawShellTextTypewriter(font, TextFormat("ENV TYPE: %s", clim.envType), wikiDividerX + 20, infoY + lineSpacing, 20, ARC_COLOR_GREEN, s->typewriterTimer, s->typewriterDuration);
                Arc_DrawShellTextTypewriter(font, TextFormat("TEMPERATURE: %s", tempStr), wikiDividerX + 20, infoY + lineSpacing*2, 20, ARC_COLOR_GREEN, s->typewriterTimer, s->typewriterDuration);
                Arc_DrawShellTextTypewriter(font, TextFormat("WEATHER: %s", clim.weather), wikiDividerX + 20, infoY + lineSpacing*3, 20, ARC_COLOR_GREEN, s->typewriterTimer, s->typewriterDuration);
                Arc_DrawShellTextTypewriter(font, TextFormat("ANOMALIES: %s", clim.anomalies), wikiDividerX + 20, infoY + lineSpacing*4, 20, Arc_GetAnomalyColor(clim.anomalyIndex), s->typewriterTimer, s->typewriterDuration);
            } else {
                char title[128];
                snprintf(title, sizeof(title), "MISSION: %s", missionNames[s->missionSelected]);
                Arc_DrawShellText(font, title, wikiDividerX + 20, rightY, 30, ARC_COLOR_GREEN);
                const char* st = (s->missionClicked == s->missionSelected) ? "SELECTED" : "NOT SELECTED";
                Arc_DrawShellText(font, TextFormat("STATUS: %s", st), wikiDividerX + 20, rightY + 50, 20, ARC_COLOR_GREEN);
            }
            EndScissorMode();

            if (s->navSelected == 0) {
                float ly = dividerY + 8 + 15 + 15 + 24 + 10 + 15;
                float lh = 24.0f;
                float emPanelX = menuDividerX - ARC_EMERGENCY_PANEL_WIDTH;
                for (int i = 0; i < ARC_LANDING_STEPS; i++) {
                    int br = get_landing_bracket(s, i);
                    const char* pre = (br == 1) ? "[1]" : (br == 0) ? "[0]" : "[-]";
                    const char* cur = (i == s->landingStepSelected) ? "> " : "  ";
                    char line[128];
                    snprintf(line, sizeof(line), "%s%s %s", cur, pre, landingStepNames[i]);
                    float rowY = ly + i * (lh + 2);
                    Arc_DrawShellText(font, line, wikiDividerX + 20, rowY, 18, ARC_COLOR_GREEN);
                    if (s->landingLoadingStep == i || s->landingDone[i]) {
                        float prog = 1.0f;
                        if (s->landingLoadingStep == i)
                            prog = ((float)GetTime() - s->landingLoadingStart) / 3.0f;
                        if (prog > 1.0f) prog = 1.0f;
                        if (s->landingShowDoneForStep == i && (float)GetTime() < s->landingShowDoneUntil)
                            Arc_DrawShellText(font, "DONE!", emPanelX - 80, rowY, 18, ARC_COLOR_GREEN);
                    }
                }
            }
        }
    } else {
        Arc_DrawShellText(font, "ARC REPORT QUOTA - Em desenvolvimento", screenW/2 - 200, screenH/2 - 20, 24, ARC_COLOR_GREEN);
        Arc_DrawShellText(font, "Use 'switch -t /priority-manage' para retornar", screenW/2 - 250, screenH/2 + 20, 18, WHITE);
    }

    DrawLineEx((Vector2){40, screenH - 75}, (Vector2){screenW - 40, screenH - 75}, 1, ARC_COLOR_GREEN);
    char prompt[128];
    snprintf(prompt, sizeof(prompt), "%s@ARC_Shell>", username && username[0] ? username : "operator");
    Arc_DrawShellText(font, prompt, 40, screenH - 60, 25, ARC_COLOR_GREEN);
    float pw = MeasureTextEx(font, prompt, 25, 2).x + 10;
    if (s->suggestionCount > 0 && s->suggestionLine[0])
        Arc_DrawShellText(font, s->suggestionLine, 40, screenH - 105, 18, (Color){120, 255, 120, 255});
    BeginScissorMode((int)(40 + pw), (int)(screenH - 75), (int)(screenW - pw - 80), 50);
    Arc_DrawShellText(font, s->command_str, 40 + pw, screenH - 60, 25, ARC_COLOR_GREEN);
    if ((int)(GetTime() * 2) % 2 == 0) {
        float cw = MeasureTextEx(font, s->command_str, 25, 2).x;
        DrawRectangle((int)(40 + pw + cw + 2), (int)(screenH - 58), 12, 22, ARC_COLOR_GREEN);
    }
    EndScissorMode();

    (void)cpuName;
    (void)totalRam;
}
