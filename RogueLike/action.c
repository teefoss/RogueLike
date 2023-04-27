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
static Direction GetDirectionToTile(World * world, TileCoord start, TileCoord end)
{
    CalculateDistances(&world->map, end, 0);

    Direction best_direction = NO_DIRECTION;
    int min_distance = INT_MAX;

    for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
        Tile * adj = GetAdjacentTile(&world->map, start, d);
        TileCoord tc = AdjacentTileCoord(start, d);

        Actor * a = GetActorAtTile(&world->actors, tc);

        // TODO: JT
        // Don't move there if:
        bool blocked =
        a // there's an actor there...
        && !a->flags.no_collision // ... that is collidable ...
        && !TileCoordsEqual(a->tile, end); //...and it not the target

        if ( !adj->flags.blocks_movement
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
void A_TargetAndChasePlayerIfVisible(Actor * actor)
{
    World * world = &actor->game->world;
    Map * map = &world->map;
    Actor * player = FindActor(&world->actors, ACTOR_PLAYER);

    if ( LineOfSight(map, actor->tile, player->tile) ) {
        actor->target_tile = player->tile;
        actor->flags.has_target = true;
    }

    if ( actor->flags.has_target ) {
        Direction d = GetDirectionToTile(world, actor->tile, actor->target_tile);
        TryMoveActor(actor, d);

        // Arrived at target?
        if ( TileCoordsEqual(actor->tile, actor->target_tile) ) {
            actor->flags.has_target = false;
        }
    }
}


void A_ChasePlayerIfVisible(Actor * actor)
{
    World * world = &actor->game->world;
    Actor * player = FindActor(&world->actors, ACTOR_PLAYER);

    if ( LineOfSight(&world->map, actor->tile, player->tile) ) {
        Direction d = GetDirectionToTile(world, actor->tile, player->tile);
        TryMoveActor(actor, d);
    }
}


void A_StupidChasePlayerIfVisible(Actor * actor)
{
    World * world = &actor->game->world;
    Actor * player = FindActor(&world->actors, ACTOR_PLAYER);

    if ( LineOfSight(&world->map, actor->tile, player->tile) ) {
        int dx = SIGN(player->tile.x - actor->tile.x);
        int dy = SIGN(player->tile.y - actor->tile.y);
        Direction d = GetDirection(dx, dy);
        TryMoveActor(actor, d);
    }
}
