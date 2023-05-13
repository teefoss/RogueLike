//
//  tile.h
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#ifndef tile_h
#define tile_h

#include "render.h"

#define MAX_LIGHT 255

typedef enum {
    TILE_FLOOR,
    TILE_TREE,
    TILE_WALL,
    TILE_DOOR,
    TILE_EXIT, // Level exit stairs
    TILE_GOLD_DOOR, // Exit room locked door
    TILE_START, // Just a floor with a symbol, to mark player's start tile
    TILE_WATER,
    TILE_TELEPORTER,
    TILE_BUTTON_NOT_PRESSED,
    TILE_BUTTON_PRESSED,

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
        bool tree_present       : 1; // In the forest, there a tree here.
    } flags;

    u8 light; // Current light level.
    s16 distance; // For pathfinding. Updated via CalculateDistances()
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
