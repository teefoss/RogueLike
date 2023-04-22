//
//  game_state.h
//  RogueLike
//
//  Created by Thomas Foster on 4/16/23.
//

#ifndef game_state_h
#define game_state_h

#include <SDL.h>
#include <stdbool.h>

#define MAX_GAME_STATES 10

typedef struct game Game;

typedef struct game_state {
    bool (* process_event)(Game *, const SDL_Event *);
    void (* update)(Game *, float dt);
    void (* render)(const Game *);

    void (* on_enter)(Game *);
    void (* on_exit)(Game *);

    int duration_ticks; // -1 indicates indefinite length.

    // If state has a finite length, next_state must not be NULL.
    const struct game_state * next_state;
} GameState;


extern const GameState level_idle;
extern const GameState level_turn;
extern const GameState intermission;


const GameState * GetGameState(const Game * game);
void PushState(Game * game, const GameState * new_state);
void PopState(Game * game);
void ChangeState(Game * game, const GameState * new_state);
void UpdateState(Game * game, float dt);

#endif /* game_state_h */
