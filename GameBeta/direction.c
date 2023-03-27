//
//  direction.c
//  GameBeta
//
//  Created by Thomas Foster on 3/27/23.
//

#include "direction.h"

//const int x_deltas[NUM_DIRECTIONS] = { 0, -1, 1, 0, -1, 1, -1, 1 };
//const int y_deltas[NUM_DIRECTIONS] = { -1, 0, 0, 1, -1, -1, 1, 1 };

static const tile_coord_t direction_deltas[NUM_DIRECTIONS] = {
    [NORTH] = { 0, -1 },
    [EAST] = { 1, 0 },
    [SOUTH] = { 0, 1 },
    [WEST] = { -1, 0 },
    [NORTH_WEST] = { -1, -1 },
    [NORTH_EAST] = { 1, -1 },
    [SOUTH_WEST] = { -1, 1 },
    [SOUTH_EAST] = { 1, 1 },
};


int XDelta(direction_t direction)
{
    if ( direction == NO_DIRECTION ) {
        return 0;
    } else {
        return direction_deltas[direction].x;
    }
}


int YDelta(direction_t direction)
{
    if ( direction == NO_DIRECTION ) {
        return 0;
    } else {
        return direction_deltas[direction].y;
    }
}


///
/// Get the tile coordinate adjacent to `coord`.
///
tile_coord_t AdjacentTileCoord(tile_coord_t coord, direction_t direction)
{
    if ( direction == NO_DIRECTION ) {
        return coord;
    } else {
        return AddTileCoords(coord, direction_deltas[direction]);
    }
}
