//
//  action.c
//  RogueLike
//
//  Created by Thomas Foster on 11/8/22.
//

#include "game.h"
#include "actor.h"
#include <limits.h>

/// Actor targets player if visible. Take one step toward target, NSEW only.
void A_ChaseBasic(actor_t * actor)
{
    map_t * map = &actor->game->map;
    actor_t * player = GetPlayer(actor->game);

    if ( LineOfSight(map, actor->tile, player->tile, false) ) {
        actor->target = player;
    }

    if ( actor->target ) {
        // Find the tile with the smallest distance to player.
        direction_t best_direction = NO_DIRECTION;
        int min_distance = INT_MAX;

        for ( direction_t d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
            tile_t * adj = GetAdjacentTile(map, actor->tile, d);
            tile_coord_t tc = AdjacentTileCoord(actor->tile, d);

            actor_t * a = GetActorAtTile(map->actors.list, map->actors.count, tc);
            bool blocked = a && !a->flags.no_collision && a != actor->target;

            if ( !adj->flags.blocking
                && adj->distance < min_distance
                && !blocked )
            {
                min_distance = adj->distance;
                best_direction = d;
            }
        }

        TryMoveActor(actor, best_direction);
    }
}
