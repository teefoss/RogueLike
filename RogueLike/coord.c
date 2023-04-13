//
//  coord.c
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#include "coord.h"
#include "mathlib.h"


tile_coord_t AddTileCoords(tile_coord_t a, tile_coord_t b)
{
    tile_coord_t result = { a.x + b.x, a.y + b.y };

    return result;
}


bool TileCoordsEqual(tile_coord_t a, tile_coord_t b)
{
    return a.x == b.x && a.y == b.y;
}


int TileDistance(tile_coord_t a, tile_coord_t b)
{
    return DISTANCE(a.x, a.y, b.x, b.y);
}
