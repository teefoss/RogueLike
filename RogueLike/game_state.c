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

bool LevelIdleProcessInput(Game * game, const SDL_Event * event);
void LevelIdleUpdate(Game * game, float dt);
void LevelIdleOnEnter(Game * game);

void LevelTurnUpdate(Game * game, float dt);
void LevelTurnOnEnter(Game * game);

void GamePlayRender(const Game * game);

void IntermissionRender(const Game * game);
void IntermissionOnExit(Game * game);

void TitleScreenRender(const Game * game);

const GameState level_idle = {
        .process_event  = LevelIdleProcessInput,
        .update         = LevelIdleUpdate,
        .render         = GamePlayRender,
        .on_enter       = LevelIdleOnEnter,
        .on_exit        = NULL,
        .next_state     = NULL,
};

const GameState level_turn = {
        .process_event  = NULL,
        .update         = LevelTurnUpdate,
        .render         = GamePlayRender,
        .on_enter       = LevelTurnOnEnter,
        .on_exit        = NULL,
        .next_state     = &level_idle,
};

const GameState intermission = {
        .process_event  = NULL,
        .update         = NULL,
        .render         = IntermissionRender,
        .on_enter       = NULL,
        .on_exit        = IntermissionOnExit,
        .finite_duration = true,
        .duration_ticks = MS2TICKS(3000, FPS),
        .next_state     = &level_idle,
};

const GameState game_state_menu = {
    .process_event = MenuProcessEvent,
    .render = MenuRender,
};

const GameState game_state_title = {
    .render = TitleScreenRender,
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

    if ( state->finite_duration ) {
        // Run timer for finite length states
        if ( --game->state_timer <= 0 ) {
            ChangeState(game, state->next_state);
        }
    }

    if ( state->update ) {
        state->update(game, dt);
    }
}
