#include "app/ui/arc_terminal/boot.h"
#include "app/ui/arc_terminal/arc_render.h"
#include <raylib.h>
#include <stdio.h>

void Arc_Boot_Render(float bootTimer, const char* cpuName, unsigned long long totalRam, Font font) {
    Arc_DrawShellText(font, "ARC BIOS v2.10 - (c) 2198", 40, 40, 20, ARC_COLOR_GREEN);
    Arc_DrawShellText(font, TextFormat("CPU: %s", cpuName ? cpuName : "UNKNOWN"), 40, 70, 20, ARC_COLOR_GREEN);
    if (bootTimer > 1.0f) {
        int ramCount = (int)((bootTimer - 1.0f) * 16);
        if ((unsigned long long)ramCount > totalRam) ramCount = (int)totalRam;
        Arc_DrawShellText(font, TextFormat("MEMORY TEST: %d GB OK", ramCount), 40, 100, 20, ARC_COLOR_GREEN);
    }
    if (bootTimer > 2.5f) {
        Arc_DrawShellText(font, "INITIALIZING CONTROL UNITS...", 40, 140, 20, ARC_COLOR_GREEN);
        if (bootTimer > 3.8f) Arc_DrawShellText(font, " > [HOST BRIDGE] READY", 60, 170, 20, ARC_COLOR_GREEN);
        if (bootTimer > 4.1f) Arc_DrawShellText(font, " > [PCI BUS] READY", 60, 200, 20, ARC_COLOR_GREEN);
        if (bootTimer > 4.4f) Arc_DrawShellText(font, " > [XGT VIDEO ADAPTER] READY", 60, 230, 20, ARC_COLOR_GREEN);
    }
    if (bootTimer > 5.0f) Arc_DrawShellText(font, "BOOTING ARC_SHELL OS...", 40, 300, 24, ARC_COLOR_GREEN);
}
