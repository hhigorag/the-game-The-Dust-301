#include "app/ui/arc_terminal/arc_common.h"

const char* Arc_GetModeTitle(ArcShellMode mode) {
    switch (mode) {
        case ARC_MODE_PRIORITY_MANAGE: return "ARC_SHELL | PRIORITY MANAGE";
        case ARC_MODE_NAVIGATION: return "ARC_SHELL | [NAVIGATION MODE] ";
        case ARC_MODE_ARC_REPORT: return "ARC_SHELL | ARC REPORT QUOTA";
        default: return "ARC_SHELL OS";
    }
}
