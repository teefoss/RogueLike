//
//  tile.h
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#ifndef tile_h
#define tile_h

#include "render.h"
#include "shorttypes.h"

#define MAX_LIGHT 255

typedef enum {
    TILE_NULL,
    TILE_FOREST_GROUND,
    TILE_DUNGEON_FLOOR,
    TILE_TREE,
    TILE_DUNGEON_WALL,
    TILE_DUNGEON_DOOR,
    TILE_FOREST_EXIT,
    TILE_DUNGEON_EXIT,
    TILE_GOLD_DOOR, // Exit room locked door
    TILE_START, // Just a floor with a symbol, to mark player's start tile
    TILE_WATER,
    TILE_TELEPORTER,
    TILE_BUTTON_NOT_PRESSED,
    TILE_BUTTON_PRESSED,
    TILE_WOODEN_FLOOR,
    TILE_WHITE_OPENING,
    TILE_WOODEN_WALL,

    NUM_TILE_TYPES,
} TileType;


typedef s8 TileID;

typedef struct {
    u8 type; // a tile_type_t
    u8 variety; // A value that can be used for visual randomization.
    s16 id; // forest region, or dungeon room (-1 if none).

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
    s16 player_distance; // "   "
    u8 tag;
} Tile;

Tile CreateTile(TileType type);

void RenderTile(const Tile * tile,
                int area,
                int signature,
                int pixel_x,
                int pixel_y,
                int render_size,
                bool debug,
                const RenderInfo * render_info);

//void DebugDrawTile(const tile_t * tile, int x, int y, int size);
const char * TileName(TileType type);

#endif /* tile_h */
