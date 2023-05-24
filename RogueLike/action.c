//
//  action.c
//  RogueLike
//
//  Created by Thomas Foster on 11/8/22.
//

#include "game.h"
#include "actor.h"
#include "astar.h"

#include "mathlib.h"

#include <limits.h>

#if 1
/// From tile `start`, find the adjacent tile with the smallest distance
/// to target tile `end`.
static Direction PathFindToTile(Map * map, TileCoord start, TileCoord end, bool player, bool diagonals)
{
//    float start_time = ProgramTime();
//    CalculateDistances(&world->map, end, 0, false);
//    printf("CalcDist: %.2f ms\n", (ProgramTime() - start_time) * 1000.0f);

    Direction best_direction = NO_DIRECTION;
    int min_distance = INT_MAX;

    int num_directions = diagonals ? NUM_DIRECTIONS : NUM_CARDINAL_DIRECTIONS;
    for ( Direction d = 0; d < num_directions; d++ ) {
        Tile * adj = GetAdjacentTile(map, start, d);
        if ( adj == NULL ) {
            continue;
        }

        TileCoord tc = AdjacentTileCoord(start, d);
        s16 distance = player ? adj->player_distance : adj->distance;
        Actor * a = GetActorAtTile(&map->actor_list, tc);

        // Don't move there if:
        bool blocked =
        a // there's an actor there...
        && ActorBlocksAll(a) // ... that is collidable ...
        && !TileCoordsEqual(a->tile, end); //...and it is not the target

        if ( !adj->flags.blocks_movement
            && distance < min_distance
            && !blocked )
        {
            min_distance = distance;
            best_direction = d;
        }
    }

    return best_direction;
}

// TODO: combine with PathFindTo
static Direction
PathFindAwayFromTile(Map * map, TileCoord subject, TileCoord away_from, bool player)
{
//    CalculateDistances(&world->map, away_from, 0, false);

    Direction best_direction = NO_DIRECTION;

    Tile * self_tile = GetTile(map, subject);
    int max_distance = self_tile->player_distance;

    for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
        Tile * adj = GetAdjacentTile(map, subject, d);
        TileCoord tc = AdjacentTileCoord(subject, d);
        s16 distance = player ? adj->player_distance : adj->distance;
        Actor * a = GetActorAtTile(&map->actor_list, tc);

        // TODO: JT
        // Don't move there if:
        bool blocked =
        a // there's an actor there...
        && ActorBlocksAll(a) // ... that is collidable ...
        && !TileCoordsEqual(a->tile, away_from); //...and it not the target

        if ( !adj->flags.blocks_movement
            && distance > max_distance
            && !blocked )
        {
            max_distance = adj->distance;
            best_direction = d;
        }
    }

    return best_direction;
}
#endif

/// Actor targets player tile if visible. Take one step toward target, NSEW only.
void A_TargetAndChasePlayerIfVisible(Actor * actor)
{
    World * world = &actor->game->world;
    Map * map = world->map;
    Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);

    if ( LineOfSight(map, actor->tile, player->tile) ) {
        actor->target_tile = player->tile;
        actor->flags.has_target = true;
    }

    if ( actor->flags.has_target ) {
        CalculateDistances(map, actor->target_tile, 0, false);
        Direction d = PathFindToTile(map, actor->tile, actor->target_tile, false, actor->info->flags.moves_diagonally);
        TileCoord coord = AdjacentTileCoord(actor->tile, d);
        TryMoveActor(actor, coord);

        // Arrived at target?
        if ( TileCoordsEqual(actor->tile, actor->target_tile) ) {
            actor->flags.has_target = false;
        }
    }
}


void A_ChasePlayerIfVisible(Actor * actor)
{
    World * world = &actor->game->world;
    Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);

    if ( LineOfSight(world->map, actor->tile, player->tile) ) {
//        Path path = FindPath(world, actor->tile, player->tile, true);
        Direction d = PathFindToTile(world->map, actor->tile, player->tile, true, actor->info->flags.moves_diagonally);
        TileCoord coord = AdjacentTileCoord(actor->tile, d);
        TryMoveActor(actor, coord);
//        if ( path.size > 0 ) {
//            TryMoveActor(actor, path.coords[path.size - 1]);
//        }
    }
}


void A_StupidChasePlayerIfVisible(Actor * actor)
{
    World * world = &actor->game->world;
    Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);

    if ( LineOfSight(world->map, actor->tile, player->tile) ) {
        int dx = SIGN(player->tile.x - actor->tile.x);
        int dy = SIGN(player->tile.y - actor->tile.y);
        Direction d = GetDirection(dx, dy);
        TileCoord coord = AdjacentTileCoord(actor->tile, d);
        TryMoveActor(actor, coord);
    }
}


// TODO: FIX THIS!
void A_SpiderChase(Actor * spider)
{
    World * world = &spider->game->world;
    Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);

    if ( LineOfSight(world->map, spider->tile, player->tile) ) {
        // Try to step in the basic direction of the player
        int dx = SIGN(player->tile.x - spider->tile.x);
        int dy = SIGN(player->tile.y - spider->tile.y);
        Direction d = GetDirection(dx, dy);
        Tile * tile = GetAdjacentTile(world->map, spider->tile, d);

        TileCoord coord;

        if ( tile->light <= world->info->revealed_light ) {
            coord = AdjacentTileCoord(spider->tile, d);
            TryMoveActor(spider, coord);
        } else {
            // Regular light
            Direction d2 = PathFindAwayFromTile(world->map, spider->tile, player->tile, true);
            coord = AdjacentTileCoord(spider->tile, d2);
            TryMoveActor(spider, coord);
        }
    }
}


#define GHOST_RADIUS 4

void A_GhostChase(Actor * ghost)
{
    World * world = &ghost->game->world;
    Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);

    if ( LineOfSight(world->map, ghost->tile, player->tile) ) {
        ghost->target_tile = player->tile;
        ghost->flags.has_target = true;
    }

    if ( ghost->flags.has_target ) {
        int distance = TileDistance(ghost->tile, ghost->target_tile);

        if ( distance > GHOST_RADIUS ) {
            // Get a list of potential tiles around the player to teleport to.
            TileCoord coords[(GHOST_RADIUS * 2 + 1) * (GHOST_RADIUS * 2 + 1)];
            int num_coords = 0;

            for ( int y = ghost->target_tile.y - GHOST_RADIUS;
                 y <= ghost->target_tile.y + GHOST_RADIUS;
                 y++ )
            {
                for ( int x = ghost->target_tile.x - GHOST_RADIUS;
                     x <= ghost->target_tile.x + GHOST_RADIUS;
                     x++ )
                {
                    // Tile distance from player
                    int tile_dist = DISTANCE(ghost->target_tile.x, ghost->target_tile.y, x, y);

                    if ( IsInBounds(world->map, x, y)
                        && tile_dist >= 1
                        && tile_dist <= GHOST_RADIUS ) {
                        coords[num_coords].x = x;
                        coords[num_coords].y = y;
                        num_coords++;
                    }
                }
            }

            // Select one and try to move there.
            int index = Random(0, num_coords - 1);
            TryMoveActor(ghost, coords[index]);

        } else {
            Direction d = PathFindToTile(world->map, ghost->tile, player->tile, true, ghost->info->flags.moves_diagonally);
            TileCoord coord = AdjacentTileCoord(ghost->tile, d);
            TryMoveActor(ghost, coord);
//            Path path = FindPath(world, ghost->tile, player->tile, false);
//            if ( path.size > 0 ) {
//                TryMoveActor(ghost, path.coords[1]);
//            }
        }
    }
}
