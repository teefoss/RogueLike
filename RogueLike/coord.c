//
//  coord.c
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#include "game.h"
#include "coord.h"
#include "mathlib.h"


TileCoord AddTileCoords(TileCoord a, TileCoord b)
{
    TileCoord result = { a.x + b.x, a.y + b.y };

    return result;
}


bool TileCoordsEqual(TileCoord a, TileCoord b)
{
    return a.x == b.x && a.y == b.y;
}


int TileDistance(TileCoord a, TileCoord b)
{
    return DISTANCE(a.x, a.y, b.x, b.y);
}


vec2_t TileCoordToScaledWorldCoord(TileCoord tile_coord, vec2_t offset)
{
    vec2_t world_coord = {
        .x = world_coord.x = tile_coord.x * SCALED(TILE_SIZE),
        .y = world_coord.y = tile_coord.y * SCALED(TILE_SIZE)
    };

    return world_coord;
}
