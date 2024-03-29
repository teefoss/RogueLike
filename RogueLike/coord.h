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

#include "vector.h"
#include "shorttypes.h"
#include "mathlib.h"

#include <stdbool.h>

typedef struct { s16 x, y; } TileCoord;

bool TileInBox(TileCoord coord, Box box);
TileCoord AddTileCoords(TileCoord a, TileCoord b);
bool TileCoordsEqual(TileCoord a, TileCoord b);
int TileDistance(TileCoord a, TileCoord b);
vec2_t TileCoordToScaledWorldCoord(TileCoord tile_coord, vec2_t offset);

#endif /* coord_h */
