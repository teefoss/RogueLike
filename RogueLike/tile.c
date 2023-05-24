//
//  tile.c
//  RogueLike
//
//  Created by Thomas Foster on 12/12/22.
//

#include "game.h"
#include "debug.h"

#include "mathlib.h"
#include "video.h"
#include "texture.h"

// Sprite sheet location.
// Tiles with multiple visible varieties are layed out horizontally
// and its location is the leftmost cell.
typedef struct tile_info tile_info_t;
static struct tile_info {
    struct { u8 x, y; } sprite_cell;
    u8 height; // in tiles, unspecified (0) is treated as 1 tile
    s8 y_offset; // in tiles
    u8 num_variants;
} _info[NUM_TILE_TYPES] = {
    [TILE_NULL] = {
        .sprite_cell = { 1, 0 },
    },
    [TILE_FOREST_GROUND] = {
        // TODO: define this
        .sprite_cell = { 0, 5 },
        .height = 2,
        .num_variants = 9,
    },
    [TILE_DUNGEON_FLOOR] = {
        .sprite_cell = { 0, 1 },
        .num_variants = 8,
    },
    [TILE_TREE] = {
        .sprite_cell = { 9, 5 },
        .num_variants = 2,
        .height = 2,
    },
    [TILE_DUNGEON_WALL] = {
        .sprite_cell = { 1, 0 },
    },
    [TILE_DUNGEON_DOOR] = {
        .sprite_cell = { 3, 3 },
    },
    [TILE_FOREST_EXIT] = {
        .sprite_cell = { 0, 5 },
    },
    [TILE_DUNGEON_EXIT] = {
        .sprite_cell = { 0, 3 },
    },
    [TILE_GOLD_DOOR] = {
        .sprite_cell = { 2, 3 },
    },
    [TILE_START] = {
        .sprite_cell = { 4, 3 },
    },
    [TILE_WATER] = {
        .sprite_cell = { 0, 7 },
        .num_variants = 8,
    },
    [TILE_TELEPORTER] = {
        .sprite_cell = { 1, 8 },
    },
    [TILE_BUTTON_NOT_PRESSED] = {
        .sprite_cell = { 6, 3 },
    },
    [TILE_BUTTON_PRESSED] = {
        .sprite_cell = { 5, 3 },
    },
    [TILE_WOODEN_FLOOR] = {
        .sprite_cell = { 0, 10 },
    },
    [TILE_WHITE_OPENING] = {
        .sprite_cell = { 1, 10 },
    },
    [TILE_WOODEN_WALL] = {
        .sprite_cell = { 2, 10 },
    }
};


static Tile tile_templates[NUM_TILE_TYPES] = {
    [TILE_NULL] = {
        .flags = { .blocks_movement = true, .blocks_sight = true },
    },
    [TILE_DUNGEON_WALL] = {
        .flags = { .blocks_movement = true, .blocks_sight = true },
    },
    [TILE_TREE] = {
        .flags = { .blocks_movement = true, .blocks_sight = true }
    },
    [TILE_DUNGEON_DOOR] = {
        .flags = { .blocks_movement = true, .blocks_sight = true },
    },
    [TILE_FOREST_EXIT] = {
        .flags = { .player_only = true, },
    },
    [TILE_DUNGEON_EXIT] = {
        .flags = { .player_only = true, },
    },
    [TILE_GOLD_DOOR] = {
        .flags = { .blocks_movement = true, .blocks_sight = true },
    },
    [TILE_TELEPORTER] = {
        .flags = { .bright = true, .player_only = true },
    },
    [TILE_WATER] = {
        .flags = { .blocks_movement = true }
    },
    [TILE_BUTTON_NOT_PRESSED] = {
        .flags = { .blocks_movement = true, .player_only = true },
    },
    [TILE_WOODEN_WALL] = {
        .flags = { .blocks_movement = true, .blocks_sight = true },
    }
};


Tile CreateTile(TileType type)
{
    Tile tile = tile_templates[type];
    tile.type = type;
    tile.variety = Random(0, 255);

    return tile;
}


const char * TileName(TileType type)
{
    switch ( type ) {
            CASE_RETURN_STRING(TILE_NULL);
            CASE_RETURN_STRING(TILE_FOREST_GROUND);
            CASE_RETURN_STRING(TILE_DUNGEON_FLOOR);
            CASE_RETURN_STRING(TILE_TREE);
            CASE_RETURN_STRING(TILE_DUNGEON_WALL);
            CASE_RETURN_STRING(TILE_DUNGEON_DOOR);
            CASE_RETURN_STRING(TILE_FOREST_EXIT);
            CASE_RETURN_STRING(TILE_DUNGEON_EXIT);
            CASE_RETURN_STRING(TILE_GOLD_DOOR);
            CASE_RETURN_STRING(TILE_START);
            CASE_RETURN_STRING(TILE_WATER);
            CASE_RETURN_STRING(TILE_TELEPORTER);
            CASE_RETURN_STRING(NUM_TILE_TYPES);
            CASE_RETURN_STRING(TILE_BUTTON_NOT_PRESSED);
            CASE_RETURN_STRING(TILE_BUTTON_PRESSED);
            CASE_RETURN_STRING(TILE_WOODEN_FLOOR);
            CASE_RETURN_STRING(TILE_WHITE_OPENING);
            CASE_RETURN_STRING(TILE_WOODEN_WALL);
    }
}


/// - parameter debug: Ignore lighting and tile's revealed property.
void RenderTile(const Tile * tile,
                int area,
                int signature,
                int pixel_x,
                int pixel_y,
                int render_size,
                bool debug,
                const RenderInfo * render_info)
{
    SDL_Texture * tiles = render_info->tile_texture;
    tile_info_t * info = &_info[tile->type];

    if ( debug || tile->flags.bright ) {
        // In debug, always draw at full light.
        SDL_SetTextureColorMod(tiles, 255, 255, 255);
    } else {
        // Apply tile's light level.
        if ( area == AREA_FOREST ) {
            SDL_SetTextureColorMod(tiles, tile->light, tile->light, 128);
        } else {
            SDL_SetTextureColorMod(tiles, tile->light, tile->light, tile->light);
        }
    }

    SDL_Rect src;
    src.w = TILE_SIZE;
    src.x = info->sprite_cell.x * TILE_SIZE;
    src.y = info->sprite_cell.y * TILE_SIZE;

    SDL_Rect dst;
    dst.x = pixel_x;
    dst.y = pixel_y + info->y_offset * render_size;
    dst.w = render_size;

    // Calculate height.
    if ( info->height ) {
        src.h = info->height * TILE_SIZE;
        dst.h = info->height * render_size;
    } else {
        src.h = TILE_SIZE;
        dst.h = render_size;
    }

    switch ( (TileType)tile->type ) {
        case TILE_DUNGEON_WALL:
            if ( area == AREA_DUNGEON ) {
                src.x = 1 * TILE_SIZE;
                src.y = 0 * TILE_SIZE;
                V_DrawTexture(tiles, &src, &dst); // Blank it to start.

                Direction draw_order[NUM_DIRECTIONS] = {
                    NORTH,
                    NORTH_WEST,
                    NORTH_EAST,
                    WEST,
                    SOUTH_WEST,
                    EAST,
                    SOUTH_EAST,
                    SOUTH
                };

                src.y = 32;
                for ( int i = 0; i < NUM_DIRECTIONS; i++ ) {
                    Direction direction = draw_order[i];

                    if ( signature & DIR_BIT(direction) ) {
                        src.x = direction * TILE_SIZE;
                        V_DrawTexture(tiles, &src, &dst);
                    }
                }
                return;
            }
            break;
        case TILE_FOREST_GROUND:
            if ( tile->variety == 012 ) {
                src.x += (info->num_variants - 1) * TILE_SIZE; // flower
            } else if ( tile->variety < 170 ) {
                src.x += (tile->variety % (info->num_variants - 1)) * TILE_SIZE;
            }
            break;
        case TILE_DUNGEON_FLOOR:
            if ( tile->variety > 112 ) {
                src.x += (tile->variety % info->num_variants) * TILE_SIZE;
            }
            break;
        case TILE_WATER:
            if ( tile->variety < 112 ) {
                src.x += (tile->variety % info->num_variants) * TILE_SIZE;
            }
            break;
        default:
            if ( info->num_variants > 1 ) {
                src.x += tile->variety % info->num_variants * TILE_SIZE;
            }
            break;
    }

    V_DrawTexture(tiles, &src, &dst);

    // Show tile distance to player
    if ( !tile->flags.blocks_movement ) {
        if ( show_distances ) {
            V_SetGray(255);
            V_PrintString(dst.x, dst.y, "%d", tile->player_distance);
        }
    }
}
