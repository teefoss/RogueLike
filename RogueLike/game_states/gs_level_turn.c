//
//  gs_level_turn.c
//  RogueLike
//
//  Created by Thomas Foster on 5/11/23.
//

#include "game.h"


void LevelTurn_OnEnter(Game * game)
{
    game->move_timer = 0.0f;
}


/// Run the move timer and do actor movement animations.
void LevelTurn_Update(Game * game, float dt)
{
    game->move_timer += 5.0f * dt;

    // End turn state?
    if ( game->move_timer >= 1.0f ) {
        // We're done.
        game->move_timer = 1.0f;
        ChangeState(game, &gs_level_idle);
        // Don't return here, make sure actors complete their animation.
    }


    // Do all actor animations.
    FOR_EACH_ACTOR(actor, game->world.map->actor_list) {
        if ( actor->animation ) {
            actor->animation(actor, game->move_timer);
            if ( game->move_timer == 1.0f ) {
                actor->animation = NULL; // Remove the animation.
            }
        }
    }

    UpdateLevel(game, dt);
}


const GameState gs_level_turn = {
    .update             = LevelTurn_Update,
    .render             = GamePlayRender,
    .on_enter           = LevelTurn_OnEnter,
};
