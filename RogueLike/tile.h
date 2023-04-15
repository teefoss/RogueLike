//
//  tile.h
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#ifndef tile_h
#define tile_h

#include "area.h"

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
} TileType;


typedef s8 TileID;

typedef struct {
    u8 type; // a tile_type_t
    u8 variety; // A value that can be used for visual randomization.
    s8 room_num; // or -1 if not in a room.

    struct {
        bool blocks_movement    : 1;
        bool blocks_sight       : 1;
        bool player_only        : 1;
        bool visible            : 1;
        bool revealed           : 1;
        bool bright             : 1;
    } flags;

    u8 light; // Current light level.
    s16 distance; // For pathfinding. Updated via CalculateDistances()
} Tile;

Tile CreateTile(TileType type);

void RenderTile(const Tile * tile,
                Area area,
                int signature,
                int pixel_x,
                int pixel_y,
                int render_size,
                bool debug);

//void DebugDrawTile(const tile_t * tile, int x, int y, int size);
const char * TileName(TileType type);

#endif /* tile_h */
