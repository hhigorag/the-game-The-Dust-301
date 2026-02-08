#include "app/ui/arc_terminal/login.h"
#include <string.h>
#include <raylib.h>

bool Arc_Login_ProcessKey(ArcLoginLogicContext* ctx, int key) {
    if (!ctx || !ctx->username || !ctx->loginLetterCount || !ctx->authError) return false;
    if (key == 10 || key == 13 || key == 257) key = KEY_ENTER;
    if (key >= 32 && key <= 125 && *ctx->loginLetterCount < 31) {
        ctx->username[(*ctx->loginLetterCount)++] = (char)key;
        ctx->username[*ctx->loginLetterCount] = '\0';
        *ctx->authError = false;
        return false;
    }
    if (key == KEY_BACKSPACE && *ctx->loginLetterCount > 0) {
        ctx->username[--(*ctx->loginLetterCount)] = '\0';
        return false;
    }
    if (key == KEY_ENTER) {
        if (strcmp(ctx->username, "operator-301") == 0) {
            return true;
        }
        *ctx->authError = true;
    }
    return false;
}
