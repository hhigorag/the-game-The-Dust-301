#include "app/ui/arc_terminal/login.h"
#include "app/ui/arc_terminal/arc_render.h"
#include <raylib.h>
#include <math.h>

void Arc_Login_Render(ArcLoginRenderContext* ctx, Font font) {
    if (!ctx) return;
    float boxWidth = 400.0f;
    float boxHeight = 150.0f;
    Rectangle loginBox = { (ctx->screenW - boxWidth) / 2.0f, (ctx->screenH - boxHeight) / 2.0f, boxWidth, boxHeight };

    DrawRectangleLinesEx(loginBox, 2, ARC_COLOR_GREEN);

    const char* loginTitle = "USER AUTHENTICATION";
    Vector2 titleSize = MeasureTextEx(font, loginTitle, 20, 1);
    Arc_DrawShellText(font, loginTitle, (ctx->screenW - titleSize.x) / 2.0f, loginBox.y + 20, 20, ARC_COLOR_GREEN);

    float promptX = loginBox.x + 30;
    float promptY = loginBox.y + 75;
    Arc_DrawShellText(font, "LOGIN ID >", promptX, promptY, 20, ARC_COLOR_GREEN);
    Arc_DrawShellText(font, ctx->username, promptX + 130, promptY, 20, ARC_COLOR_GREEN);

    if (((int)(GetTime() * 2) % 2 == 0)) {
        float textWidth = MeasureTextEx(font, ctx->username, 20, 1).x;
        DrawRectangle((int)(promptX + 132 + textWidth), (int)promptY + 2, 10, 18, ARC_COLOR_GREEN);
    }

    if (ctx->authError) {
        const char* errorMsg = "ACCESS DENIED";
        Vector2 errorSize = MeasureTextEx(font, errorMsg, 16, 1);
        Arc_DrawShellText(font, errorMsg, (ctx->screenW - errorSize.x) / 2.0f, loginBox.y + boxHeight + 15, 16, WHITE);
    }
}
