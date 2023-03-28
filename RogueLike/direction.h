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
} direction_t;

int XDelta(direction_t direction);
int YDelta(direction_t direction);
tile_coord_t AdjacentTileCoord(tile_coord_t coord, direction_t direction);

#endif /* direction_h */
