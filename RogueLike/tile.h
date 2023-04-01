//
//  tile.h
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#ifndef tile_h
#define tile_h

#define MAX_LIGHT 255

typedef enum {
    TILE_FLOOR,
    TILE_WALL,
    TILE_DOOR,
    TILE_EXIT, // Level exit stairs
    TILE_GOLD_DOOR, // Exit room locked door
    TILE_START, // Just a floor with a symbol, to mark player's start tile

    NUM_TILE_TYPES,
} tile_type_t;


typedef s8 tile_id_t;

typedef struct {
    u8 type; // a tile_type_t
    u8 variety; // A value that can be used for visual randomization.
    u8 num_variants; // Visual
    s8 room_num; // or -1 if not in a room.

    struct {
        u8 blocking     : 1;
        u8 player_only  : 1;
        u8 visible      : 1;
        u8 revealed     : 1;
    } flags;

    u8 light; // Current light level.
    u8 light_target; // Used to lerp (light -> light_target).
    s16 distance; // For pathfinding. Updated via UpdateDistanceMap()
} tile_t;

tile_t CreateTile(tile_type_t type);

void RenderTile(const tile_t * tile,
                int signature,
                int pixel_x,
                int pixel_y,
                int scale, // TODO: scale is the wrong term
                bool debug);

void DebugDrawTile(const tile_t * tile, int x, int y, int size);

#endif /* tile_h */
