//
//  tile.h
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#ifndef tile_h
#define tile_h

#include "level.h"

#define MAX_LIGHT 255

typedef enum {
    TILE_FLOOR,
    TILE_WALL,
    TILE_DOOR,
    TILE_EXIT, // Level exit stairs
    TILE_GOLD_DOOR, // Exit room locked door
    TILE_START, // Just a floor with a symbol, to mark player's start tile
    TILE_WATER,
    TILE_TELEPORTER,

    NUM_TILE_TYPES,
} tile_type_t;


typedef s8 tile_id_t;

typedef struct {
    u8 type; // a tile_type_t
    u8 variety; // A value that can be used for visual randomization.
    s8 room_num; // or -1 if not in a room.

    struct {
        u8 blocking     : 1;
        u8 player_only  : 1;
        u8 visible      : 1;
        u8 revealed     : 1;
        u8 bright       : 1;
    } flags;

    u8 light; // Current light level.
    s16 distance; // For pathfinding. Updated via CalculateDistances()
} tile_t;

tile_t CreateTile(tile_type_t type);

void RenderTile(const tile_t * tile,
                area_t area,
                int signature,
                int pixel_x,
                int pixel_y,
                int render_size,
                bool debug);

//void DebugDrawTile(const tile_t * tile, int x, int y, int size);
const char * TileName(tile_type_t type);

#endif /* tile_h */
