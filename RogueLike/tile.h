//
//  tile.h
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#ifndef tile_h
#define tile_h

typedef enum {
    TILE_FLOOR,
    TILE_WALL,
    TILE_DOOR,
    TILE_EXIT,
    TILE_GOLD_DOOR,
    TILE_START,
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

    // Sprite sheet location.
    // Tiles with multiple visible varieties are layed out horizontally
    // and its location is the leftmost cell.
    struct { u8 x, y; } sprite_cell;
} tile_t;

tile_t CreateTile(tile_type_t type);

#endif /* tile_h */
