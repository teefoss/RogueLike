//
//  direction.c
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#include "direction.h"

static const TileCoord deltas[NUM_DIRECTIONS] = {
    [NORTH] = { 0, -1 },
    [EAST] = { 1, 0 },
    [SOUTH] = { 0, 1 },
    [WEST] = { -1, 0 },
    [NORTH_WEST] = { -1, -1 },
    [NORTH_EAST] = { 1, -1 },
    [SOUTH_WEST] = { -1, 1 },
    [SOUTH_EAST] = { 1, 1 },
};


int XDelta(Direction direction)
{
    if ( direction == NO_DIRECTION ) {
        return 0;
    } else {
        return deltas[direction].x;
    }
}


int YDelta(Direction direction)
{
    if ( direction == NO_DIRECTION ) {
        return 0;
    } else {
        return deltas[direction].y;
    }
}


Direction OppositeDirection(Direction direction)
{
    switch ( direction ) {
        case NORTH:         return SOUTH;
        case WEST:          return EAST;
        case EAST:          return WEST;
        case SOUTH:         return NORTH;
        case NORTH_WEST:    return SOUTH_EAST;
        case NORTH_EAST:    return SOUTH_WEST;
        case SOUTH_WEST:    return NORTH_EAST;
        case SOUTH_EAST:    return NORTH_WEST;
        default:            return NO_DIRECTION;
    }
}


Direction GetDirection(int dx, int dy)
{
    for ( Direction d = 0; d < NUM_DIRECTIONS; d++ ) {
        if ( deltas[d].x == dx && deltas[d].y == dy ) {
            return d;
        }
    }

    return NO_DIRECTION;
}


///
/// Get the tile coordinate adjacent to `coord`.
///
TileCoord AdjacentTileCoord(TileCoord coord, Direction direction)
{
    if ( direction == NO_DIRECTION ) {
        return coord;
    } else {
        return AddTileCoords(coord, deltas[direction]);
    }
}
