#ifndef MATCH_STATE_H
#define MATCH_STATE_H

// Estados do jogo
typedef enum {
    MATCH_STATE_MENU,
    MATCH_STATE_LOBBY,
    MATCH_STATE_INGAME,
    MATCH_STATE_PAUSED,
    MATCH_STATE_RESULTS
} MatchState;

// Retorna o estado atual do jogo
MatchState MatchState_Get(void);

// Define o estado do jogo
void MatchState_Set(MatchState state);

#endif // MATCH_STATE_H
