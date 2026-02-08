/*
 * Shell logic: estado, update, processamento de teclas e mouse.
 * Reimportado de terminal-with-raylib com adaptações para overlay 1600x920.
 */
#include "app/ui/arc_terminal/shell.h"
#include "app/ui/arc_terminal/shell_private.h"
#include "app/ui/arc_terminal/arc_common.h"
#include "app/ui/arc_terminal/arc_utils.h"
#include "app/ui/arc_clim.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <raylib.h>

#define MAX_LINES SHELL_MAX_LINES
#define MAX_LOG_LINES SHELL_MAX_LOG_LINES
#define MAX_CMD_HISTORY SHELL_MAX_CMD_HISTORY
#define K_COMMAND_COUNT 5

static const char* kCommands[] = { "help", "clear", "echo", "sysinfo", "switch" };
#define PLANET_COUNT 5
#define MISSION_COUNT 4

static void shell_add_to_terminal(ArcShellContext* s, const char* text) {
    if (!s || !text) return;
    size_t len = arc_strnlen(text, 255);
    if (s->lineCount < MAX_LINES) {
        memcpy(s->terminalHistory[s->lineCount], text, len);
        s->terminalHistory[s->lineCount][len] = '\0';
        s->lineCount++;
    } else {
        for (int i = 0; i < MAX_LINES - 1; i++)
            memcpy(s->terminalHistory[i], s->terminalHistory[i + 1], sizeof(s->terminalHistory[i]));
        memcpy(s->terminalHistory[MAX_LINES - 1], text, len);
        s->terminalHistory[MAX_LINES - 1][len] = '\0';
    }
}

static void shell_add_to_log(ArcShellContext* s, const char* text) {
    if (!s || !text) return;
    size_t len = arc_strnlen(text, 255);
    if (s->logCount < MAX_LOG_LINES) {
        memcpy(s->logHistory[s->logCount], text, len);
        s->logHistory[s->logCount][len] = '\0';
        s->logCount++;
    } else {
        for (int i = 0; i < MAX_LOG_LINES - 1; i++)
            memcpy(s->logHistory[i], s->logHistory[i + 1], sizeof(s->logHistory[i]));
        memcpy(s->logHistory[MAX_LOG_LINES - 1], text, len);
        s->logHistory[MAX_LOG_LINES - 1][len] = '\0';
    }
}

static void shell_push_cmd_history(ArcShellContext* s, const char* cmd) {
    if (!s || !cmd || !cmd[0]) return;
    if (s->cmdHistCount > 0 && strcmp(s->commandHistory[s->cmdHistCount - 1], cmd) == 0) return;
    if (s->cmdHistCount < MAX_CMD_HISTORY) {
        size_t len = arc_strnlen(cmd, sizeof(s->commandHistory[0]) - 1);
        memcpy(s->commandHistory[s->cmdHistCount], cmd, len);
        s->commandHistory[s->cmdHistCount][len] = '\0';
        s->cmdHistCount++;
    } else {
        for (int i = 0; i < MAX_CMD_HISTORY - 1; i++)
            memcpy(s->commandHistory[i], s->commandHistory[i + 1], sizeof(s->commandHistory[0]));
        size_t len = arc_strnlen(cmd, sizeof(s->commandHistory[0]) - 1);
        memcpy(s->commandHistory[MAX_CMD_HISTORY - 1], cmd, len);
        s->commandHistory[MAX_CMD_HISTORY - 1][len] = '\0';
    }
}

static int get_landing_bracket(const ArcShellContext* s, int step) {
    if (step == 0) return s->landingDone[0] ? 1 : 0;
    if (step == 1) return s->landingDone[1] ? 1 : (s->landingDone[0] ? 0 : -1);
    if (step == 2) return s->landingDone[2] ? 1 : (s->landingDone[1] ? 0 : -1);
    if (step == 3) return s->landingDone[3] ? 1 : (s->landingDone[2] ? 0 : -1);
    if (step == 4) return s->landingDone[4] ? 1 : (s->landingDone[3] ? 0 : -1);
    return -1;
}

static void shell_execute_command(ArcShellContext* s, const char* raw,
    const char* username, const char* cpuName, unsigned long long totalRam, int resW, int resH) {
    if (!s || !raw) return;
    char buf[1024];
    strncpy(buf, raw, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    arc_strtrim_left(buf);
    if (buf[0] == '\0') return;

    char* cmd = buf;
    char* args = NULL;
    for (char* p = buf; *p; p++) {
        if ((unsigned char)*p <= 32) { *p = '\0'; args = p + 1; break; }
    }
    if (args) arc_strtrim_left(args);
    arc_strlower(cmd);

    shell_add_to_log(s, TextFormat("CMD: %s", raw));

    if (strcmp(cmd, "help") == 0) {
        shell_add_to_terminal(s, "--- | ARC_SHELL COMMAND DATABASE | ---");
        shell_add_to_terminal(s, " > help - lista comandos");
        shell_add_to_terminal(s, " > clear - clear terminal cmd");
        shell_add_to_terminal(s, " > echo <txt> - impress text");
        shell_add_to_terminal(s, " > sysinfo - show system information");
        shell_add_to_terminal(s, " > switch -t <target> - muda modo do sistema");
        shell_add_to_terminal(s, "   targets: /navigation-mode, /arc-report, /priority-manage");
        return;
    }
    if (strcmp(cmd, "clear") == 0) { shell_add_to_log(s, "OK: clear"); return; }
    if (strcmp(cmd, "echo") == 0) {
        if (args && args[0]) { shell_add_to_terminal(s, args); }
        else { shell_add_to_terminal(s, "echo: faltou texto. uso: echo <texto>"); }
        return;
    }
    if (strcmp(cmd, "sysinfo") == 0) {
        shell_add_to_terminal(s, TextFormat("USER: %s", username && username[0] ? username : "UNKNOWN"));
        shell_add_to_terminal(s, TextFormat("CPU: %s", cpuName && cpuName[0] ? cpuName : "UNKNOWN"));
        shell_add_to_terminal(s, TextFormat("RAM: %llu GB", totalRam));
        shell_add_to_terminal(s, TextFormat("RES: %dx%d", resW, resH));
        return;
    }
    if (strcmp(cmd, "switch") == 0) {
        if (!args || !args[0]) {
            shell_add_to_terminal(s, "switch: faltou target. uso: switch -t <target>");
            return;
        }
        char* target = args;
        if (strncmp(target, "-t", 2) == 0) {
            target = args + 2;
            while (*target && (unsigned char)*target <= 32) target++;
        }
        if (strncmp(target, "/navigation-mode", 16) == 0) {
            s->currentMode = ARC_MODE_NAVIGATION;
            s->navSelected = 0;
            shell_add_to_terminal(s, "MODE SWITCHED: NAVIGATION MODE");
        } else if (strncmp(target, "/arc-report", 11) == 0) {
            s->currentMode = ARC_MODE_ARC_REPORT;
            shell_add_to_terminal(s, "MODE SWITCHED: ARC REPORT QUOTA");
        } else if (strncmp(target, "/priority-manage", 17) == 0) {
            s->currentMode = ARC_MODE_PRIORITY_MANAGE;
            shell_add_to_terminal(s, "MODE SWITCHED: PRIORITY MANAGE");
        } else {
            shell_add_to_terminal(s, TextFormat("switch: target desconhecido '%s'", target));
        }
        return;
    }
    shell_add_to_terminal(s, TextFormat("comando desconhecido: %s (digite 'help')", cmd));
}

static int arc_starts_with_ci(const char* s, const char* prefix) {
    if (!s || !prefix) return 0;
    while (*prefix) {
        char a = (char)tolower((unsigned char)*s++);
        char b = (char)tolower((unsigned char)*prefix++);
        if (a != b) return 0;
        if (a == '\0') return 0;
    }
    return 1;
}

static int arc_has_space(const char* s) {
    if (!s) return 0;
    for (; *s; s++) if ((unsigned char)*s <= 32) return 1;
    return 0;
}

static void build_suggestions(ArcShellContext* s) {
    s->suggestionLine[0] = '\0';
    s->suggestionCount = 0;
    const char* input = s->command_str;
    if (!input || !input[0]) return;

    if (arc_starts_with_ci(input, "switch -t")) {
        const char* targets[] = { "/navigation-mode", "/arc-report", "/priority-manage" };
        const char* afterSwitch = input + strlen("switch -t");
        while (*afterSwitch && (unsigned char)*afterSwitch <= 32) afterSwitch++;
        const char* partial = (*afterSwitch == '/') ? afterSwitch + 1 : "";
        int matches = 0;
        size_t used = 0;
        for (int i = 0; i < 3; i++) {
            const char* tn = targets[i] + 1;
            if (arc_starts_with_ci(tn, partial) || partial[0] == '\0') {
                matches++;
                if (matches == 1) {
                    strcpy(s->suggestionLine, "<suggestions>:");
                    used = strlen(s->suggestionLine);
                }
                if (used + strlen(targets[i]) + 4 < sizeof(s->suggestionLine)) {
                    if (matches > 1) { s->suggestionLine[used++] = '|'; s->suggestionLine[used++] = ' '; }
                    size_t n = strlen(targets[i]);
                    memcpy(s->suggestionLine + used, targets[i], n);
                    used += n;
                    s->suggestionLine[used] = '\0';
                }
            }
        }
        s->suggestionCount = matches;
        return;
    }
    if (arc_has_space(input)) return;
    int matches = 0;
    size_t used = 0;
    for (int i = 0; i < K_COMMAND_COUNT; i++) {
        if (arc_starts_with_ci(kCommands[i], input)) {
            matches++;
            if (matches == 1) {
                strcpy(s->suggestionLine, "<suggestions>:");
                used = strlen(s->suggestionLine);
            }
            if (used + strlen(kCommands[i]) + 4 < sizeof(s->suggestionLine)) {
                if (matches > 1) { s->suggestionLine[used++] = '|'; s->suggestionLine[used++] = ' '; }
                size_t n = strlen(kCommands[i]);
                memcpy(s->suggestionLine + used, kCommands[i], n);
                used += n;
                s->suggestionLine[used] = '\0';
            }
        }
    }
    s->suggestionCount = matches;
}

static void apply_autocomplete(ArcShellContext* s) {
    if (!s->command_str[0]) return;
    if (arc_starts_with_ci(s->command_str, "switch -t")) {
        const char* targets[] = { "/navigation-mode", "/arc-report", "/priority-manage" };
        const char* afterSwitch = s->command_str + strlen("switch -t");
        while (*afterSwitch && (unsigned char)*afterSwitch <= 32) afterSwitch++;
        const char* partial = (*afterSwitch == '/') ? afterSwitch + 1 : "";
        const char* matches[4];
        int m = 0;
        for (int i = 0; i < 3 && m < 4; i++) {
            if (arc_starts_with_ci(targets[i] + 1, partial) || partial[0] == '\0')
                matches[m++] = targets[i];
        }
        if (m == 0) return;
        if (m == 1) {
            snprintf(s->command_str, sizeof(s->command_str), "switch -t %s", matches[0]);
            s->shellLetterCount = (int)strlen(s->command_str);
        } else if (m > 1) {
            char common[64];
            strncpy(common, matches[0] + 1, sizeof(common) - 1);
            common[sizeof(common)-1] = '\0';
            for (int j = 1; j < m; j++) {
                size_t k = 0;
                while (common[k] && matches[j][k+1] && tolower((unsigned char)common[k]) == tolower((unsigned char)matches[j][k+1])) k++;
                common[k] = '\0';
            }
            if (common[0]) {
                snprintf(s->command_str, sizeof(s->command_str), "switch -t /%s", common);
                s->shellLetterCount = (int)strlen(s->command_str);
            }
        }
        return;
    }
    if (arc_has_space(s->command_str)) return;
    const char* matches[16];
    int m = 0;
    for (int i = 0; i < K_COMMAND_COUNT && m < 16; i++) {
        if (arc_starts_with_ci(kCommands[i], s->command_str)) matches[m++] = kCommands[i];
    }
    if (m == 0) return;
    if (m == 1) {
        strncpy(s->command_str, matches[0], 1023);
        s->command_str[1023] = '\0';
        s->shellLetterCount = (int)strlen(s->command_str);
    } else {
        char common[64];
        strncpy(common, matches[0], sizeof(common) - 1);
        common[sizeof(common)-1] = '\0';
        for (int j = 1; j < m; j++) {
            size_t k = 0;
            while (common[k] && matches[j][k] && tolower((unsigned char)common[k]) == tolower((unsigned char)matches[j][k])) k++;
            common[k] = '\0';
        }
        if (common[0]) {
            strncpy(s->command_str, common, 1023);
            s->command_str[1023] = '\0';
            s->shellLetterCount = (int)strlen(s->command_str);
        }
    }
}

ArcShellContext* Arc_Shell_Create(void) {
    ArcShellContext* s = (ArcShellContext*)calloc(1, sizeof(ArcShellContext));
    if (!s) return NULL;
    s->menuItems[0] = "system"; s->menuItems[1] = "fuel"; s->menuItems[2] = "energy";
    s->menuItems[3] = "scan"; s->menuItems[4] = "cargo"; s->menuItems[5] = "log";
    s->menuCount = 6;
    s->menuEnabled[0] = s->menuEnabled[1] = s->menuEnabled[2] = true;
    s->menuEnabled[3] = s->menuEnabled[4] = false;
    s->menuEnabled[5] = true;
    s->typewriterDuration = 0.2f;
    s->lastNavSelected = -1;
    s->lastMenuSelected = -1;
    s->lastPlanetSelected = -1;
    s->cmdHistNav = -1;
    return s;
}

void Arc_Shell_Destroy(ArcShellContext* s) {
    free(s);
}

void Arc_Shell_Reset(ArcShellContext* s, const char* username) {
    if (!s) return;
    s->lineCount = 0;
    s->command_str[0] = '\0';
    s->shellLetterCount = 0;
    s->logCount = 0;
    s->logScroll = 0;
    s->cmdHistCount = 0;
    s->cmdHistNav = -1;
    s->menuSelected = 0;
    s->navSelected = 0;
    s->planetSelected = 0;
    s->currentMode = ARC_MODE_NAVIGATION;
    s->lastCurrentMode = ARC_MODE_PRIORITY_MANAGE;
    s->typewriterActive = true;
    s->typewriterTimer = 0;
    s->wikiScroll = 0;
    s->missionSelected = 0;
    s->missionClicked = -1;
    memset(s->landingDone, 0, sizeof(s->landingDone));
    s->landingStepSelected = 0;
    s->landingLoadingStep = -1;
    s->landingShowDoneForStep = -1;
    s->mapUpdating = 0;
    s->locationLandUpdating = 0;
    s->doorsOpenUpdating = 0;
    s->openAirlockSwitchActive = 0;
    s->landingEmergencySelected = -1;
    s->emergencyConfirming = false;
    s->landingHeaderTypewriter = 0;
    (void)username;
}

void Arc_Shell_Update(ArcShellContext* s, float screenW, float screenH,
    int mouseLocalX, int mouseLocalY, bool mouseLeftPressed, float mouseWheel, float dt) {
    if (!s) return;

    float dividerY = screenH * (ARC_DIVIDER_Y_OFFSET / 1080.0f);
    float menuDividerX = screenW - 375.0f;
    float wikiDividerX = 40.0f + 300.0f;

    if (s->currentMode == ARC_MODE_PRIORITY_MANAGE) {
        if (s->menuSelected != s->lastMenuSelected || s->currentMode != s->lastCurrentMode) {
            s->typewriterActive = true;
            s->typewriterTimer = 0;
            s->lastMenuSelected = s->menuSelected;
            s->lastCurrentMode = s->currentMode;
        }
    }

    if (s->currentMode == ARC_MODE_NAVIGATION) {
        if (s->navSelected >= 2) s->navSelected = 0;

        float menuAreaWidth = (screenW - 40.0f) - menuDividerX;
        if (mouseLeftPressed && mouseLocalX >= 0 && mouseLocalY >= 0) {
            Vector2 mouse = { (float)mouseLocalX, (float)mouseLocalY };
            for (int i = 0; i < 2; i++) {
                float itemY = 160.0f + (i * 45.0f);
                Rectangle r = { menuDividerX + 10, itemY - 5, menuAreaWidth - 20, 32 };
                if (CheckCollisionPointRec(mouse, r)) {
                    s->navSelected = i;
                    break;
                }
            }

            float lineSpacing = 30.0f;
            float leftContentY = 140.0f;
            float listStartY = leftContentY + (lineSpacing * 2.5f);
            if (s->navSelected == 0 && mouseLocalX >= 40 && mouseLocalX < wikiDividerX && mouseLocalY >= 115) {
                for (int i = 0; i < PLANET_COUNT; i++) {
                    float itemY = listStartY + (i * lineSpacing);
                    if (mouseLocalY >= itemY - 5 && mouseLocalY < itemY + 20) {
                        s->planetSelected = i;
                        s->typewriterActive = true;
                        s->typewriterTimer = 0;
                        break;
                    }
                }
            } else if (s->navSelected == 1 && mouseLocalX >= 40 && mouseLocalX < wikiDividerX && mouseLocalY >= 115) {
                for (int i = 0; i < MISSION_COUNT; i++) {
                    float itemY = listStartY + (i * lineSpacing);
                    if (mouseLocalY >= itemY - 5 && mouseLocalY < itemY + 20) {
                        s->missionSelected = i;
                        break;
                    }
                }
            } else if (s->navSelected == 0 && mouseLocalY >= dividerY + 8 && mouseLocalX >= wikiDividerX + 20 && mouseLocalX < menuDividerX) {
                float ly = dividerY + 8 + 15 + 24 + 8 + 15 + 15 + 6*24 + 10 + 15;
                float lh = 24.0f;
                for (int i = 0; i < ARC_LANDING_STEPS; i++) {
                    float rowY = ly + i * (lh + 2);
                    if (mouseLocalY >= rowY - 5 && mouseLocalY < rowY + lh) {
                        s->landingStepSelected = i;
                        break;
                    }
                }
            }
        }

        if (s->navSelected != s->lastNavSelected || s->currentMode != s->lastCurrentMode) {
            s->typewriterActive = true;
            s->typewriterTimer = 0;
            s->lastNavSelected = s->navSelected;
            s->lastCurrentMode = s->currentMode;
        }
        if (s->lastPlanetSelected != s->planetSelected && s->navSelected == 0) {
            s->typewriterActive = true;
            s->typewriterTimer = 0;
        }
        s->lastPlanetSelected = s->planetSelected;
        s->lastMenuSelected = s->menuSelected;

        if (s->typewriterActive) {
            s->typewriterTimer += dt;
            if (s->typewriterTimer >= s->typewriterDuration) {
                s->typewriterTimer = s->typewriterDuration;
                s->typewriterActive = false;
            }
        }

        if (mouseWheel != 0 && s->navSelected == 0) {
            float planetWikiViewportH = dividerY - 115.0f;
            float totalContentHeight = 140.0f + 30.0f + 20.0f + (30.0f * 6) + 25.0f + 15.0f + (30.0f * 6) + 20.0f;
            float maxScroll = (totalContentHeight > planetWikiViewportH) ? (totalContentHeight - planetWikiViewportH) : 0.0f;
            if (mouseLocalX >= wikiDividerX && mouseLocalX < menuDividerX && mouseLocalY >= 115 && mouseLocalY <= 115 + planetWikiViewportH) {
                s->wikiScroll -= mouseWheel * 20.0f;
                if (s->wikiScroll < 0) s->wikiScroll = 0;
                if (s->wikiScroll > maxScroll) s->wikiScroll = maxScroll;
            }
        }
        if (mouseWheel != 0 && mouseLocalX >= 40 && mouseLocalX < wikiDividerX && mouseLocalY >= 115 && mouseLocalY < screenH - 75) {
            float listStartY = 140 + 75;
            float listHeight = (s->navSelected == 0 ? PLANET_COUNT : MISSION_COUNT) * 30.0f;
            if (mouseLocalY >= listStartY && mouseLocalY < listStartY + listHeight) {
                if (s->navSelected == 0) {
                    s->planetSelected = (mouseWheel > 0) ? (s->planetSelected - 1 + PLANET_COUNT) % PLANET_COUNT : (s->planetSelected + 1) % PLANET_COUNT;
                    s->typewriterActive = true;
                    s->typewriterTimer = 0;
                } else {
                    s->missionSelected = (mouseWheel > 0) ? (s->missionSelected - 1 + MISSION_COUNT) % MISSION_COUNT : (s->missionSelected + 1) % MISSION_COUNT;
                }
            }
        }

        if (s->emergencyConfirming) {
            s->emergencyConfirmTimer -= dt;
            if (s->emergencyConfirmTimer < 0) s->emergencyConfirmTimer = 0;
        }

        if (s->landingLoadingStep >= 0 && (float)GetTime() - s->landingLoadingStart >= 3.0f) {
            int st = s->landingLoadingStep;
            s->landingDone[st] = 1;
            if (st == 2) { s->mapUpdating = 1; s->mapUpdatingStart = (float)GetTime(); }
            if (st == 3) { s->locationLandUpdating = 1; s->locationLandUpdatingStart = (float)GetTime(); }
            if (st == 4) { s->doorsOpenUpdating = 1; s->doorsOpenUpdatingStart = (float)GetTime(); }
            s->landingLoadingStep = -1;
            s->landingShowDoneForStep = st;
            s->landingShowDoneUntil = (float)GetTime() + 5.0f;
        }
        if (s->landingShowDoneForStep >= 0 && (float)GetTime() > s->landingShowDoneUntil) {
            if (s->landingShowDoneForStep == 4) {
                s->openAirlockSwitchActive = 1;
                s->openAirlockSwitchStart = (float)GetTime();
            }
            s->landingShowDoneForStep = -1;
        }
        if (s->mapUpdating && (float)GetTime() - s->mapUpdatingStart >= 2.0f) s->mapUpdating = 0;
        if (s->locationLandUpdating && (float)GetTime() - s->locationLandUpdatingStart >= 2.0f) s->locationLandUpdating = 0;
        if (s->doorsOpenUpdating && (float)GetTime() - s->doorsOpenUpdatingStart >= 2.0f) s->doorsOpenUpdating = 0;
        if (s->openAirlockSwitchActive && (float)GetTime() - s->openAirlockSwitchStart >= 3.0f) {
            s->currentMode = ARC_MODE_PRIORITY_MANAGE;
            s->openAirlockSwitchActive = 0;
        }
    }

    /* Atualiza clima para Planet Wiki */
    if (s->currentMode == ARC_MODE_NAVIGATION && s->navSelected == 0) {
        int h = (int)(GetTime() / 3600) % 24;
        int m = (int)(GetTime() / 60) % 60;
        int sec = (int)GetTime() % 60;
        UpdateClimate(h, m, sec, s->planetSelected, dt);
    }

    build_suggestions(s);
}

static int norm_key(int key) {
    return (key == 10 || key == 13 || key == 257) ? KEY_ENTER : key;
}

void Arc_Shell_ProcessKey(ArcShellContext* s, int key,
    const char* username, const char* cpuName, unsigned long long totalRam, int resW, int resH) {
    if (!s) return;
    key = norm_key(key);

    if (s->currentMode == ARC_MODE_PRIORITY_MANAGE) {
        if (key == KEY_RIGHT) s->menuSelected = (s->menuSelected + 1) % s->menuCount;
        if (key == KEY_LEFT) s->menuSelected = (s->menuSelected - 1 + s->menuCount) % s->menuCount;
    }

    if (s->menuSelected == s->menuCount - 1) {
        if (key == KEY_PAGE_UP) { s->logScroll += 3; if (s->logScroll > s->logCount) s->logScroll = s->logCount; }
        if (key == KEY_PAGE_DOWN) { s->logScroll -= 3; if (s->logScroll < 0) s->logScroll = 0; }
    }

    if (s->currentMode != ARC_MODE_NAVIGATION) {
        if (key == KEY_UP && s->cmdHistCount > 0) {
            if (s->cmdHistNav == -1) s->cmdHistNav = s->cmdHistCount - 1;
            else if (s->cmdHistNav > 0) s->cmdHistNav--;
            strncpy(s->command_str, s->commandHistory[s->cmdHistNav], 1023);
            s->command_str[1023] = '\0';
            s->shellLetterCount = (int)strlen(s->command_str);
        }
        if (key == KEY_DOWN && s->cmdHistCount > 0) {
            if (s->cmdHistNav >= 0 && s->cmdHistNav < s->cmdHistCount - 1) {
                s->cmdHistNav++;
                strncpy(s->command_str, s->commandHistory[s->cmdHistNav], 1023);
                s->command_str[1023] = '\0';
                s->shellLetterCount = (int)strlen(s->command_str);
            } else {
                s->cmdHistNav = -1;
                s->command_str[0] = '\0';
                s->shellLetterCount = 0;
            }
        }
    }

    if (key == KEY_TAB) { apply_autocomplete(s); return; }

    if (s->currentMode == ARC_MODE_NAVIGATION && s->navSelected == 0 && (key == '1' || key == '2' || key == '3')) {
        s->landingEmergencySelected = key - '1';
        s->emergencyConfirming = false;
        s->emergencyConfirmTimer = 0;
        return;
    }

    if (key >= 32 && key <= 125 && s->shellLetterCount < 1023) {
        s->command_str[s->shellLetterCount++] = (char)key;
        s->command_str[s->shellLetterCount] = '\0';
        s->cmdHistNav = -1;
        return;
    }
    if (key == KEY_BACKSPACE && s->shellLetterCount > 0) {
        s->command_str[--s->shellLetterCount] = '\0';
        s->cmdHistNav = -1;
        return;
    }

    if (s->currentMode == ARC_MODE_NAVIGATION && s->navSelected == 0 && key == KEY_ENTER && s->shellLetterCount == 0) {
        if (s->landingEmergencySelected >= 0) {
            if (!s->emergencyConfirming) {
                s->emergencyConfirming = true;
                s->emergencyConfirmTimer = 1.0f;
            } else if (s->emergencyConfirmTimer <= 0) {
                const char* emNames[] = {"ABORT MISSION","RETURN TO ORBIT","DEPART PLANET"};
                shell_add_to_log(s, TextFormat("EMERGENCY: %s", emNames[s->landingEmergencySelected]));
                shell_add_to_terminal(s, TextFormat(">> %s CONFIRMED", emNames[s->landingEmergencySelected]));
                s->emergencyConfirming = false;
                s->landingEmergencySelected = -1;
            }
        } else {
            int br = get_landing_bracket(s, s->landingStepSelected);
            if (br == 0 && s->landingLoadingStep < 0) {
                s->landingLoadingStep = s->landingStepSelected;
                s->landingLoadingStart = (float)GetTime();
                if (s->landingStepSelected == 1) s->deployScannersWikiStart = (float)GetTime();
            }
        }
        return;
    }

    if (key == KEY_ENTER && s->shellLetterCount > 0) {
        if (strcmp(s->command_str, "clear") == 0) {
            s->lineCount = 0;
        } else {
            shell_add_to_terminal(s, TextFormat("%s@ARC_Shell> %s", username ? username : "operator", s->command_str));
            shell_push_cmd_history(s, s->command_str);
            shell_execute_command(s, s->command_str, username, cpuName, totalRam, resW, resH);
        }
        s->command_str[0] = '\0';
        s->shellLetterCount = 0;
        s->cmdHistNav = -1;
    }

    if (s->currentMode == ARC_MODE_NAVIGATION && s->navSelected == 0) {
        if (key == KEY_LEFT) s->landingStepSelected = (s->landingStepSelected - 1 + ARC_LANDING_STEPS) % ARC_LANDING_STEPS;
        if (key == KEY_RIGHT) s->landingStepSelected = (s->landingStepSelected + 1) % ARC_LANDING_STEPS;
    }
}

