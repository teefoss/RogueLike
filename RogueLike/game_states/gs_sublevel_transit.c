//
//  gs_sublevel_transit.c
//  RogueLike
//
//  Created by Thomas Foster on 5/22/23.
//
//  Handle going into and out of a sublevel. This is a bit of a hack to make
//  fading in/out work right because we would not otherwise be changing
//  state to do this.

#include <stdio.h>
#include "game_state.h"
#include "game.h"

void SublevelTransit_OnEnter(Game * game)
{
    if ( game->player_info.level_state == LEVEL_ENTER_SUB ) {
        game->player_info.level_state = LEVEL_SUB;
        LoadLevel(game, ENTER_SUBLEVEL, true);
    } else if ( game->player_info.level_state == LEVEL_EXIT_SUB ) {
        game->player_info.level_state = LEVEL_UPPER;
        LoadLevel(game, EXIT_SUBLEVEL, true);
    }

    StartFadeIn(&game->fade_state, 0.5f);
    ChangeState(game, &gs_level_idle);
}

const GameState gs_sublevel_enter = {
    .render = GamePlayRender,
    .on_enter = SublevelTransit_OnEnter,
};
