#ifndef ARC_TERMINAL_SHELL_PRIVATE_H
#define ARC_TERMINAL_SHELL_PRIVATE_H

#include "shell.h"
#include "arc_common.h"
#include <stdbool.h>

#define SHELL_MAX_LINES 20
#define SHELL_MAX_LOG_LINES 200
#define SHELL_MAX_CMD_HISTORY 50

struct ArcShellContext {
    char terminalHistory[SHELL_MAX_LINES][256];
    int lineCount;
    char command_str[1024];
    int shellLetterCount;
    char logHistory[SHELL_MAX_LOG_LINES][256];
    int logCount;
    int logScroll;
    char commandHistory[SHELL_MAX_CMD_HISTORY][1024];
    int cmdHistCount;
    int cmdHistNav;
    const char* menuItems[6];
    int menuCount;
    bool menuEnabled[6];
    int menuSelected;
    float typewriterTimer;
    float typewriterDuration;
    bool typewriterActive;
    int lastNavSelected;
    int lastMenuSelected;
    int lastPlanetSelected;
    int navSelected;
    float wikiScroll;
    int planetSelected;
    int landingDone[ARC_LANDING_STEPS];
    int landingStepSelected;
    int landingLoadingStep;
    float landingLoadingStart;
    int landingShowDoneForStep;
    float landingShowDoneUntil;
    int mapUpdating;
    float mapUpdatingStart;
    int locationLandUpdating;
    float locationLandUpdatingStart;
    int doorsOpenUpdating;
    float doorsOpenUpdatingStart;
    float deployScannersWikiStart;
    int openAirlockSwitchActive;
    float openAirlockSwitchStart;
    int landingEmergencySelected;
    float emergencyConfirmTimer;
    bool emergencyConfirming;
    float landingHeaderTypewriter;
    int missionSelected;
    int missionClicked;
    ArcShellMode currentMode;
    ArcShellMode lastCurrentMode;
    char suggestionLine[512];
    int suggestionCount;
};

#endif
