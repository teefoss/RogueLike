//
//  coord.h
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//
//  Coordinate types, arithmetic, and conversions.
//

#ifndef coord_h
#define coord_h

#include "inttypes.h"

typedef struct { s16 x, y; } tile_coord_t;

tile_coord_t AddTileCoords(tile_coord_t a, tile_coord_t b);

#endif /* coord_h */
