//
//  map.c
//  RogueLike
//
//  Created by Thomas Foster on 11/4/22.
//

#include "game.h"

#include "mathlib.h"
#include "video.h"
#include "texture.h"
#include "debug.h"

#define NUM_TILE_SPRITES 15

int tile_signatures[NUM_TILE_SPRITES] = {
    [ 0] = 255,
    [ 1] = 2,
    [ 2] = 0,
    [ 3] = 206,
    [ 4] = 87,
    [ 5] = 171,
    [ 6] = 223,
    [ 7] = 239,
    [ 8] = 138,
    [ 9] = 70,
    [10] = 127,
    [11] = 191,
    [12] = 2,
    [13] = 70,
    [14] = 138,
};

int tile_masks[NUM_TILE_SPRITES] = {
    [ 0] = 255,
    [ 1] = 14,
    [ 2] = 2,
    [ 3] = 207,
    [ 4] = 95,
    [ 5] = 175,
    [ 6] = 255,
    [ 7] = 255,
    [ 8] = 234,
    [ 9] = 214,
    [10] = 255,
    [11] = 255,
    [12] = 194,
    [13] = 79,
    [14] = 143,
};

int CalculateWallSignature(const map_t * map, tile_coord_t coord, bool ignore_reveal)
{
    int signature = 0;

    for ( direction_t i = 0; i < NUM_DIRECTIONS; i++ ) {
        tile_t * adjacent = GetAdjacentTile((map_t *)map, coord, i);

        if ( adjacent ) {
            bool is_floor = true;

            if ( adjacent->flags.blocking ) {
                is_floor = false;
            }

            if ( !ignore_reveal && !adjacent->flags.revealed ) {
                is_floor = false;
            }

            if ( is_floor ) {
                signature |= 1 << i; // Flag if there's a floor there.
            }
        }
    }

    return signature;
}

bool IsInBounds(const map_t * map, int x, int y)
{
    return x >= 0 && y >= 0 && x < map->width && y < map->height;
}


tile_t * GetAdjacentTile(map_t * map, tile_coord_t coord, direction_t direction)
{
    if ( direction == NO_DIRECTION ) {
        return GetTile(map, coord);
    }

    return GetTile(map, AdjacentTileCoord(coord, direction));
}


tile_t * GetTile(map_t * map, tile_coord_t coord)
{
    if ( !IsInBounds(map, coord.x, coord.y) ) {
        return NULL;
    }

    return &map->tiles[coord.y * map->width + coord.x];
}

tile_coord_t GetCoordinate(const map_t * map, int index)
{
    tile_coord_t coord = { index % map->width, index / map->width };
    return coord;
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

    src.x = tile->sprite_cell.x * TILE_SIZE;
    src.y = tile->sprite_cell.y * TILE_SIZE;

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
#if 0
            src.x = NUM_TILE_SPRITES * TILE_SIZE; // 'Missing tile' texture.
            for ( int i = 0; i < NUM_TILE_SPRITES; i++ ) {
                if ( (signature & tile_masks[i]) == tile_signatures[i] ) {
                    src.x = (tile->sprite_cell.x + i) * TILE_SIZE;
                    break;
                }
            }
#endif
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

static const int half_w = (GAME_WIDTH - SCALED(TILE_SIZE)) / 2;
static const int half_h = (GAME_HEIGHT - SCALED(TILE_SIZE)) / 2;

vec2_t GetRenderOffset(const actor_t * player)
{
    vec2_t offset = {
        .x = (player->tile.x * SCALED(TILE_SIZE) + player->offset_current.x) - half_w,
        .y = (player->tile.y * SCALED(TILE_SIZE) + player->offset_current.y) - half_h,
    };

    return offset;
}

void RenderMap(const game_t * game)
{
    vec2_t offset = game->camera;
    const actors_t * actors = &game->map.actors;

    //
    // Draw all tiles.
    //

    box_t vis_rect = GetVisibleRegion(&game->map, GetPlayer((actors_t *)actors));

    tile_coord_t coord;
    for ( coord.y = vis_rect.min.y; coord.y <= vis_rect.max.y; coord.y++ ) {
        for ( coord.x = vis_rect.min.x; coord.x <= vis_rect.max.x; coord.x++ ) {
            const tile_t * tile = GetTile((map_t *)&game->map, coord);

            int signature = CalculateWallSignature(&game->map, coord, false);

            int pixel_x = (coord.x * SCALED(TILE_SIZE)) - offset.x;
            int pixel_y = (coord.y * SCALED(TILE_SIZE)) - offset.y;

            RenderTile(tile, signature, pixel_x, pixel_y, SCALED(TILE_SIZE), false);

            if ( show_debug_info && TileCoordsEqual(coord, game->mouse_tile) ) {
                SDL_Rect highlight = { pixel_x, pixel_y, SCALED(TILE_SIZE), SCALED(TILE_SIZE) };
                V_SetRGB(255, 80, 80);
                V_DrawRect(&highlight);
            }
        }
    }

    //
    // Draw all actors.
    //

    // Make a list of visible actors.
    const actor_t * visible_actors[MAX_ACTORS];
    int num_visible_actors = 0;

    for ( int i = 0; i < actors->count; i++ ) {
        const actor_t * actor = &actors->list[i];
        const tile_t * tile = GetTile((map_t *)&game->map, actor->tile);

        if ( tile->flags.visible
            && actor->tile.x >= vis_rect.min.x
            && actor->tile.x <= vis_rect.max.x
            && actor->tile.y >= vis_rect.min.y
            && actor->tile.y <= vis_rect.max.y )
        {
            visible_actors[num_visible_actors++] = actor;
        }
    }

    // Sort the list by y position.
    for ( int i = 0; i < num_visible_actors; i++ ) {
        for ( int j = i + 1; j < num_visible_actors; j++ ) {
            if ( visible_actors[j]->tile.y < visible_actors[i]->tile.y ) {
                SWAP(visible_actors[j], visible_actors[i]);
            }
        }
    }

    for ( int i = 0; i < num_visible_actors; i++ ) {
        RenderActor(visible_actors[i], offset.x, offset.y);
    }
}


bool LineOfSight(map_t * map, tile_coord_t t1, tile_coord_t t2, bool reveal)
{
    int dx = abs(t2.x - t1.x);
    int dy = -abs(t2.y - t1.y);
    int sx = t1.x < t2.x ? 1 : -1;
    int sy = t1.y < t2.y ? 1 : -1;
    int err = dx + dy;
    int e2;

    while ( true ) {
        tile_t * tile = GetTile(map, t1);

        if ( reveal ) {
            tile->flags.visible = true;
            tile->flags.revealed = true;

            // For floor tiles, also reveal any surrounding wall tiles.
            if ( !tile->flags.blocking ) {
                for ( direction_t d = 0; d < NUM_DIRECTIONS; d++ ) {
                    tile_t * adjacent = GetAdjacentTile(map, t1, d);
                    adjacent->flags.visible = true;
                    adjacent->flags.revealed = true;
                }
            }
        }

        if ( t1.x == t2.x && t1.y == t2.y ) {
            return true;
        }

        if ( tile->flags.blocking ) {
            return false;
        }

        e2 = 2 * err;

        if ( e2 >= dy ) {
            err += dy;
            t1.x += sx;
        }

        if ( e2 <= dx ) {
            err += dx;
            t1.y += sy;
        }
    }
}

static bool HLineIsClear(map_t * map, int y, int x0, int x1)
{
    int x = x0;

    // Walk along the x axis.
    while ( x != x1 ) {
        tile_t * tile = GetTile(map, (tile_coord_t){ x, y });
        
        if ( tile == NULL ) {
            return false;
        }

        if ( tile->flags.blocking ) {
            return false;
        }

        if ( x < x1 ) {
            x++;
        } else if ( x > x1 ) {
            x--;
        }
    }

    return true;
}

static bool VLineIsClear(map_t * map, int x, int y0, int y1)
{
    int y = y0;

    while ( y != y1 ) {
        tile_t * tile = GetTile(map, (tile_coord_t){ x, y });
        if ( tile->flags.blocking ) {
            return false;
        }

        if ( y < y1 ) {
            y++;
        } else if ( y > y1 ) {
            y--;
        }
    }

    return true;
}



/*
 . . . . . . .
 . * * * * 1 .
 . * . . . * .
 . 0 * * * * .
 . . . . . . .
 */
bool ManhattenPathsAreClear(map_t * map, int x0, int y0, int x1, int y1)
{
    return
    ( HLineIsClear(map, y0, x0, x1) || VLineIsClear(map, x1, y0, y1) )
    &&
    ( HLineIsClear(map, y1, x0, x1) || VLineIsClear(map, x0, y0, y1) );
}


///
/// Get the rectangular region around the player that is currently visible
/// on screen.
///
box_t GetVisibleRegion(const map_t * map, const actor_t * player)
{
    int w = GAME_WIDTH / SCALED(TILE_SIZE);
    int h = GAME_HEIGHT / SCALED(TILE_SIZE);

    // Include a padding of 1 so tiles don't disappear when scrolling.

    box_t region;
    region.min.x = MAX(0, (player->tile.x - w / 2) - 1);
    region.min.y = MAX(0, (player->tile.y - h / 2) - 1);
    region.max.x = MIN((player->tile.x + w / 2) + 1, map->width - 1);
    region.max.y = MIN((player->tile.y + h / 2) + 1, map->height - 1);

    return region;
}


///
/// See if tile is adjacent to a another tile of given type.
/// - parameter coord: The tile's coordinate.
/// - parameter y: The tile's y.
/// - parameter type: The tile type to check for.
/// - parameter num_direction: The directions to check. Either
///   `NUM_CARDINAL_DIRECTIONS` (4) or `NUM_DIRECTIONS` (8).
///
bool TileIsAdjacentTo(const map_t * map,
                      tile_coord_t coord,
                      tile_type_t type,
                      int num_directions)
{
    for ( direction_t d = 0; d < num_directions; d++ ) {
        const tile_t * check = GetAdjacentTile((map_t *)map, coord, d);
        if ( check->type == type ) {
            return true;
        }
    }

    return false;
}


#pragma mark - DISTANCE MAP

typedef struct {
    tile_t * tile;
    tile_coord_t coord;
} qtile_t;

static qtile_t * queue;
static int queue_size;
static int head, tail;

static void Put(qtile_t qtile)
{
    queue[tail++] = qtile;

    if ( tail > queue_size ) {
        tail = 0;
    }
}

static qtile_t Get(void)
{
    qtile_t qtile = queue[head++];

    if ( head > queue_size ) {
        head = 0;
    }

    return qtile;
}

void FreeDistanceMapQueue(void)
{
    if ( queue ) {
        free(queue);
    }
}

/// For all walkable tiles, update tile `distance` property
/// with distance to x, y.
/// - parameter coord: The tile from which distances are calculated.
/// - parameter ignore: The tile types to be ignored, as bit flags.
void UpdateDistanceMap(map_t * map, tile_coord_t coord, int ignore_flags)
{
    queue_size = 0;

    tile_coord_t c;
    for ( c.y = 0; c.y < map->height; c.y++ ) {
        for ( c.x = 0; c.x < map->width; c.x++ ) {
            tile_t * tile = GetTile(map, c);

            if ( (ignore_flags & FLAG(tile->type)) || !tile->flags.blocking ) {
                queue_size++;
                tile->distance = -1;
            }
        }
    }

    FreeDistanceMapQueue();
    queue = calloc(queue_size, sizeof(*queue));
    head = 0;
    tail = 0;

    tile_t * tile = GetTile(map, coord);
    tile->distance = 0;
    qtile_t start = { tile, coord };
    Put(start);

    while ( head != tail ) {
        qtile_t qtile = Get();
        int distance = qtile.tile->distance;

        for ( int d = 0; d < NUM_DIRECTIONS; d++ ) {
            tile_t * edge = GetAdjacentTile(map, qtile.coord, d);
            tile_coord_t edge_coord = AdjacentTileCoord(qtile.coord, d);

            if ( edge // inbounds
                && edge->distance == -1 // not yet visited
                && ( (ignore_flags & FLAG(tile->type)) || !tile->flags.blocking ) )
            {
                // open and not yet visited
                edge->distance = distance + 1;
                qtile_t qtile = { edge, edge_coord };
                Put(qtile);
            }
        }
    }
}
