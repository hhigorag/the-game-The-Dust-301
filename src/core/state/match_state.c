#include "core/state/match_state.h"

static MatchState g_currentState = MATCH_STATE_MENU;

MatchState MatchState_Get(void) {
    return g_currentState;
}

void MatchState_Set(MatchState state) {
    g_currentState = state;
}
