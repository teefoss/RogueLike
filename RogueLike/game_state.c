//
//  game_state.c
//  RogueLike
//
//  Created by Thomas Foster on 4/16/23.
//

#include "game_state.h"
#include "game.h"
#include "menu.h"

#include "genlib.h"


const GameState gs_menu = {
    .process_event = MenuProcessEvent,
    .render = MenuRender,
};



const GameState * GetGameState(const Game * game)
{
    return game->state_stack[game->state_stack_top];
}


void PushState(Game * game, const GameState * new_state)
{
    if ( game->state_stack_top + 1 >= MAX_GAME_STATES ) {
        printf("Ran out of room in game state stack, "
               "please increase MAX_GAME_STATES!\n");
        return;
    }

    if ( new_state == NULL ) {
        Error("tried to push NULL game state!");
    }

    game->state_stack[++game->state_stack_top] = new_state;
}


void PopState(Game * game)
{
    if ( game->state_stack_top == -1 ) {
        printf("popped empty game state stack!\n");
        return;
    }

    --game->state_stack_top;
}


void FadeOutAndChangeState(Game * game,
                           const GameState * new_state,
                           float fade_duration_sec)
{
    game->post_fade_state = new_state;

    game->fade_state.type = FADE_OUT;
    game->fade_state.duration_sec = fade_duration_sec;
    game->fade_state.timer = 0.0f;
}


void ChangeStateAndFadeIn(Game * game,
                          const GameState * new_state,
                          float fade_duration_sec)
{
    ChangeState(game, new_state);
    game->post_fade_state = NULL;

    game->fade_state.type = FADE_IN;
    game->fade_state.duration_sec = fade_duration_sec;
    game->fade_state.timer = 0.0f;
}


void ChangeState(Game * game, const GameState * new_state)
{
    const GameState ** state = &game->state_stack[game->state_stack_top];

    if ( *state && (*state)->on_exit ) {
        (*state)->on_exit(game);
    }

    *state = new_state;

    if ( *state != NULL ) {
        if ( (*state)->on_enter ) {
            (*state)->on_enter(game);
        }

        if ( (*state)->finite_duration ) {
            game->state_timer = (*state)->duration_ticks;
        }
    }
}


void UpdateState(Game * game, float dt)
{
    const GameState * state = game->state_stack[game->state_stack_top];

    // Either do a fade or run the state timer
    if ( game->fade_state.type != FADE_NONE ) {
        FadeState * fs = &game->fade_state;

        // Run the fade timer.
        fs->timer += dt * (1.0f / fs->duration_sec);

        if ( fs->timer >= 1.0f ) {
            fs->type = FADE_NONE;
            fs->timer = 0.0f;
            if ( game->post_fade_state ) {
                ChangeState(game, game->post_fade_state);
                game->post_fade_state = NULL;
            }
        }
    } else if ( state->finite_duration ) {
        // Run timer for finite length states.

        if ( --game->state_timer <= 0 ) {
            ChangeState(game, state->next_state);
        }
    }

    if ( state->update ) {
        state->update(game, dt);
    }
}
