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

// A fade out is started with FadeOutAndChangeState.
// A fade in in started with ChangeStateAndFade in, or via StartFadeIn
// once the new state is active, usually in on_enter.
typedef enum {
    FADE_NONE,
    FADE_IN,
    FADE_OUT
} Fade;

typedef struct game Game;

typedef struct game_state {
    bool (* process_event)(Game *, const SDL_Event *);
    void (* update)(Game *, float dt);
    void (* render)(const Game *);

    void (* on_enter)(Game *);
    void (* on_exit)(Game *);

    bool finite_duration;
    int duration_ticks;

    // If state has a finite duration, next_state must not be NULL.
    const struct game_state * next_state;
} GameState;

extern const GameState gs_death_screen;
extern const GameState gs_intermission;
extern const GameState gs_level_idle;
extern const GameState gs_level_turn;
extern const GameState gs_menu;
extern const GameState gs_title_screen;


const GameState * GetGameState(const Game * game);
void PushState(Game * game, const GameState * new_state);
void PopState(Game * game);
void ChangeState(Game * game, const GameState * new_state);
void ChangeStateAndFadeIn(Game * game,
                          const GameState * new_state,
                          float fade_duration_sec);
void FadeOutAndChangeState(Game * game,
                           const GameState * new_state,
                           float fade_duration_sec);
void UpdateState(Game * game, float dt);

#endif /* game_state_h */
