//
//  action.c
//  RogueLike
//
//  Created by Thomas Foster on 11/8/22.
//

#include "game.h"
#include "actor.h"

#include "mathlib.h"

#include <limits.h>

/// From tile `start`, find the adjacent tile with the smallest distance
/// to target tile `end`.
static direction_t
GetDirectionToTile(map_t * map,
                   tile_coord_t start,
                   tile_coord_t end)
{
    CalculateDistances(map, end, 0);

    direction_t best_direction = NO_DIRECTION;
    int min_distance = INT_MAX;

    for ( direction_t d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
        tile_t * adj = GetAdjacentTile(map, start, d);
        tile_coord_t tc = AdjacentTileCoord(start, d);

        actor_t * a = GetActorAtTile(map->actors.list, map->actors.count, tc);

        // TODO: JT
        // Don't move there if:
        bool blocked =
        a // there's an actor there...
        && !a->flags.no_collision // ... that is collidable ...
        && !TileCoordsEqual(a->tile, end); //...and it not the target

        if ( !adj->flags.blocking
            && adj->distance < min_distance
            && !blocked )
        {
            min_distance = adj->distance;
            best_direction = d;
        }
    }

    return best_direction;
}


/// Actor targets player tile if visible. Take one step toward target, NSEW only.
void A_TargetAndChasePlayerIfVisible(actor_t * actor)
{
    map_t * map = &actor->game->map;
    actor_t * player = GetPlayer(actor->game);

    if ( LineOfSight(map, actor->tile, player->tile) ) {
        actor->target_tile = player->tile;
        actor->flags.has_target = true;
    }

    if ( actor->flags.has_target ) {
        direction_t d = GetDirectionToTile(map, actor->tile, actor->target_tile);
        TryMoveActor(actor, d);

        // Arrived at target?
        if ( TileCoordsEqual(actor->tile, actor->target_tile) ) {
            actor->flags.has_target = false;
        }
    }
}


void A_ChasePlayerIfVisible(actor_t * actor)
{
    map_t * map = &actor->game->map;
    actor_t * player = GetPlayer(actor->game);

    if ( LineOfSight(map, actor->tile, player->tile) ) {
        direction_t d = GetDirectionToTile(map, actor->tile, player->tile);
        TryMoveActor(actor, d);
    }
}


void A_StupidChasePlayerIfVisible(actor_t * actor)
{
    map_t * map = &actor->game->map;
    actor_t * player = GetPlayer(actor->game);

    if ( LineOfSight(map, actor->tile, player->tile) ) {
        int dx = SIGN(player->tile.x - actor->tile.x);
        int dy = SIGN(player->tile.y - actor->tile.y);
        direction_t d = GetDirection(dx, dy);
        TryMoveActor(actor, d);
    }
}
