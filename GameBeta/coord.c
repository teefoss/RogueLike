//
//  coord.c
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#include "coord.h"

tile_coord_t AddTileCoords(tile_coord_t a, tile_coord_t b)
{
    tile_coord_t result = { a.x + b.x, a.y + b.y };

    return result;
}
