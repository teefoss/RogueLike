//
//  astar.h
//  RogueLike
//
//  Created by Thomas Foster on 5/18/23.
//

#ifndef astar_h
#define astar_h

#include "world.h"

#define PATH_MAX_COORDS 256

typedef struct {
    TileCoord coords[PATH_MAX_COORDS];
    int size;
} Path;

Path FindPath(World * world, TileCoord start, TileCoord end, bool diagonal);

#endif /* astar_h */
