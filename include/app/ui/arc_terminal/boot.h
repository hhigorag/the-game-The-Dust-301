#ifndef ARC_TERMINAL_BOOT_H
#define ARC_TERMINAL_BOOT_H

#include "arc_common.h"

void Arc_Boot_Update(float* bootTimer, ArcTerminalState* outNextState, float dt);
void Arc_Boot_Render(float bootTimer, const char* cpuName, unsigned long long totalRam, Font font);

#endif
