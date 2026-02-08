#include "app/ui/arc_terminal/boot.h"

void Arc_Boot_Update(float* bootTimer, ArcTerminalState* outNextState, float dt) {
    if (!bootTimer || !outNextState) return;
    *outNextState = ARC_STATE_BOOT;
    *bootTimer += dt;
    if (*bootTimer > 7.0f) {
        *outNextState = ARC_STATE_LOGIN;
    }
}
