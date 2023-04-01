//
//  tile.c
//  RogueLike
//
//  Created by Thomas Foster on 12/12/22.
//

#include "game.h"
#include "mathlib.h"
#include "video.h"
#include "texture.h"

// Sprite sheet location.
// Tiles with multiple visible varieties are layed out horizontally
// and its location is the leftmost cell.
struct {
    u8 x;
    u8 y;
} sprite_cells[NUM_TILE_TYPES] = {
    [TILE_FLOOR]        = { 0, 1 },
    [TILE_WALL]         = { 0, 0 },
    [TILE_DOOR]         = { 3, 3 },
    [TILE_EXIT]         = { 0, 3 },
    [TILE_GOLD_DOOR]    = { 2, 3 },
    [TILE_START]        = { 4, 3 },
};


static tile_t tile_templates[] = {
    [TILE_FLOOR] = { .num_variants = 8, },
    [TILE_WALL] = { .flags = { .blocking = true }, },
    [TILE_DOOR] = { .flags = { .blocking = true }, },
    [TILE_EXIT] = { .flags = { .player_only = true }, },
    [TILE_GOLD_DOOR] = { .flags = { .blocking = true }, },
};


tile_t CreateTile(tile_type_t type)
{
    tile_t tile = tile_templates[type];
    tile.type = type;
    tile.variety = Random(0, 255);

    return tile;
}


/// - parameter debug: Ignore lighting and tile's revealed property.
void RenderTile(const tile_t * tile,
                int signature,
                int pixel_x,
                int pixel_y,
                int scale, // TODO: scale is the wrong term
                bool debug)
{
    SDL_Texture * tiles = GetTexture("assets/tiles2.png");

    SDL_Rect src, dst;
    src.w = src.h = TILE_SIZE;
    dst.w = dst.h = scale;
    dst.x = pixel_x;
    dst.y = pixel_y;

    if ( debug ) {
        // In debug, always draw at full light.
        SDL_SetTextureColorMod(tiles, 255, 255, 255);
    } else {
        // Apply tile's light level.
        SDL_SetTextureColorMod(tiles, tile->light, tile->light, tile->light);
    }

    src.x = sprite_cells[tile->type].x * TILE_SIZE;
    src.y = sprite_cells[tile->type].y * TILE_SIZE;

    switch ( (tile_type_t)tile->type ) {
        case TILE_WALL:
            src.x = 0;
            src.y = 0;
            V_DrawTexture(tiles, &src, &dst); // Blank it to start.

            direction_t draw_order[NUM_DIRECTIONS] = {
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
                direction_t direction = draw_order[i];

                if ( signature & FLAG(direction) ) {
                    src.x = direction * TILE_SIZE;
                    V_DrawTexture(tiles, &src, &dst);
                }
            }
            return;
        case TILE_FLOOR:
            if ( tile->variety < 112 ) {
                src.x += (tile->variety % 8) * TILE_SIZE;
            }
            break;
        default:
            break;
    }

    V_DrawTexture(tiles, &src, &dst);

    // F1 - show tile distance to player
    if ( !tile->flags.blocking ) {
        const u8 * keys = SDL_GetKeyboardState(NULL);
        if ( keys[SDL_SCANCODE_F1] ) {
            V_SetGray(255);
            V_PrintString(dst.x, dst.y, "%d", tile->distance);
        }
    }
}


void DebugDrawTile(const tile_t * tile, int x, int y, int size)
{
    SDL_Texture * tiles = GetTexture("assets/tiles2.png");
    SDL_SetTextureColorMod(tiles, 255, 255, 255);

    SDL_Rect src = { .w = TILE_SIZE, .h = TILE_SIZE };
    SDL_Rect dst = { .w = size, .h = size };

    if ( tile->type == TILE_WALL ) {
        src.x = 0;
        src.y = 0;
    } else {
        src.x = sprite_cells[tile->type].x * TILE_SIZE;
        src.y = sprite_cells[tile->type].y * TILE_SIZE;
        if ( tile->num_variants ) {
            src.x += (tile->variety % tile->num_variants) * TILE_SIZE;
        }
    }

    dst.x = x * size;
    dst.y = y * size;

    V_DrawTexture(tiles, &src, &dst);
}
