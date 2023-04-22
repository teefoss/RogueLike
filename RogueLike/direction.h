//
//  direction.h
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#ifndef direction_h
#define direction_h

#include "coord.h"

#define NUM_CARDINAL_DIRECTIONS 4
#define DIR_BIT(direction) (1 << direction)

typedef enum {
    NO_DIRECTION = -1,
    NORTH,
    WEST,
    EAST,
    SOUTH,
    NORTH_WEST,
    NORTH_EAST,
    SOUTH_WEST,
    SOUTH_EAST,
    NUM_DIRECTIONS,
} Direction;

int XDelta(Direction direction);
int YDelta(Direction direction);
Direction GetDirection(int dx, int dy);
TileCoord AdjacentTileCoord(TileCoord coord, Direction direction);

#endif /* direction_h */
