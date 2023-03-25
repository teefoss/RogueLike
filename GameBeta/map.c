//
//  map.c
//  GameBeta
//
//  Created by Thomas Foster on 11/4/22.
//

#include "main.h"

#include "mathlib.h"
#include "video.h"
#include "texture.h"

const int x_deltas[NUM_DIRECTIONS] = { 0, -1, 1, 0, -1, 1, -1, 1 };
const int y_deltas[NUM_DIRECTIONS] = { -1, 0, 0, 1, -1, -1, 1, 1 };

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

int CalculateWallSignature(const map_t * map, int x, int y, bool ignore_reveal)
{
    int signature = 0;

    for ( direction_t i = 0; i < NUM_DIRECTIONS; i++ ) {
        int x2 = x + x_deltas[i];
        int y2 = y + y_deltas[i];

        if ( IsInBounds(map, x2, y2) ) {
            const tile_t * tile = GetTile((map_t *)map, x2, y2);

            bool is_floor = true;

            if ( tile->flags & FLAG(TILE_BLOCKING) ) {
                is_floor = false;
            }

            if ( !ignore_reveal && !tile->revealed ) {
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

tile_t * GetAdjacentTile(map_t * map, int x, int y, direction_t direction)
{
    tile_t * tile = GetTile(map,
                            x + x_deltas[direction],
                            y + y_deltas[direction]);
    return tile;
}

tile_t * GetTile(map_t * map, int x, int y)
{
    if ( !IsInBounds(map, x, y) ) {
        return NULL;
    }

    return &map->tiles[y * map->width + x];
}

SDL_Point GetCoordinate(const map_t * map, int index)
{
    SDL_Point coord = { index % map->width, index / map->width };
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
    if ( !(tile->flags & FLAG(TILE_BLOCKING)) ) {
        const u8 * keys = SDL_GetKeyboardState(NULL);
        if ( keys[SDL_SCANCODE_F1] ) {
            V_SetGray(255);
            V_PrintString(dst.x, dst.y, "%d", tile->distance);
        }
    }
}

static const int half_w = (GAME_WIDTH - RENDER_TILE_SIZE) / 2;
static const int half_h = (GAME_HEIGHT - RENDER_TILE_SIZE) / 2;

vec2_t GetRenderOffset(const actor_t * player)
{
    vec2_t offset = {
        .x = (player->x * RENDER_TILE_SIZE + player->offset_current.x) - half_w,
        .y = (player->y * RENDER_TILE_SIZE + player->offset_current.y) - half_h,
    };

    return offset;
}

void RenderMap(const game_t * game)
{
    vec2_t offset = game->camera;

    //
    // Draw all tiles.
    //

    box_t visible_region = GetVisibleRegion(&game->map, &game->actors[0]);

    for ( int y = visible_region.min.y; y <= visible_region.max.y; y++ ) {
        for ( int x = visible_region.min.x; x <= visible_region.max.x; x++ ) {
            const tile_t * tile = GetTile((map_t *)&game->map, x, y);
            int signature = CalculateWallSignature(&game->map, x, y, false);
            int pixel_x = (x * RENDER_TILE_SIZE) - offset.x;
            int pixel_y = (y * RENDER_TILE_SIZE) - offset.y;
            RenderTile(tile, signature, pixel_x, pixel_y, RENDER_TILE_SIZE, false);
        }
    }

    //
    // Draw all actors.
    //

    // Make a list of visible actors.
    const actor_t * visible_actors[MAX_ACTORS];
    int num_visible_actors = 0;

    for ( int i = 0; i < game->num_actors; i++ ) {
        const actor_t * actor = &game->actors[i];
        const tile_t * tile = GetTile((map_t *)&game->map, actor->x, actor->y);

        if ( tile->visible
            && actor->x >= visible_region.min.x
            && actor->x <= visible_region.max.x
            && actor->y >= visible_region.min.y
            && actor->y <= visible_region.max.y )
        {
            visible_actors[num_visible_actors++] = actor;
        }
    }

    // Sort the list by y position.
    for ( int i = 0; i < num_visible_actors; i++ ) {
        for ( int j = i + 1; j < num_visible_actors; j++ ) {
            if ( visible_actors[j]->y < visible_actors[i]->y ) {
                SWAP(visible_actors[j], visible_actors[i]);
            }
        }
    }

    for ( int i = 0; i < num_visible_actors; i++ ) {
        RenderActor(visible_actors[i], offset.x, offset.y);
    }
}


bool LineOfSight(game_t * game, int x1, int y1, int x2, int y2, bool reveal)
{
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while ( true ) {
        tile_t * tile = GetTile(&game->map, x1, y1);

        if ( reveal ) {
            tile->visible = true;
            tile->revealed = true;

            // For floor tiles, also reveal any surrounding wall tiles.
            if ( !(tile->flags & FLAG(TILE_BLOCKING)) ) {
                for ( direction_t d = 0; d < NUM_DIRECTIONS; d++ ) {
                    int x3 = x_deltas[d] + x1;
                    int y3 = y_deltas[d] + y1;
                    tile_t * adjacent = GetTile(&game->map, x3, y3);
                    adjacent->visible = true;
                    adjacent->revealed = true;
                }
            }
        }

        if ( x1 == x2 && y1 == y2 ) {
            return true;
        }

        if ( tile->flags & FLAG(TILE_BLOCKING) ) {
            return false;
        }

        e2 = 2 * err;

        if ( e2 >= dy ) {
            err += dy;
            x1 += sx;
        }

        if ( e2 <= dx ) {
            err += dx;
            y1 += sy;
        }
    }
}

static bool HLineIsClear(map_t * map, int y, int x0, int x1)
{
    int x = x0;

    // Walk along the x axis.
    while ( x != x1 ) {
        tile_t * tile = GetTile(map, x, y);
        
        if ( tile == NULL ) {
            return false;
        }

        if ( tile->flags & FLAG(TILE_BLOCKING) ) {
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
        tile_t * tile = GetTile(map, x, y);
        if ( tile->flags & FLAG(TILE_BLOCKING) ) {
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
    int w = GAME_WIDTH / RENDER_TILE_SIZE;
    int h = GAME_HEIGHT / RENDER_TILE_SIZE;

    // Include a padding of 1 so tiles don't disappear when scrolling.

    box_t region;
    region.min.x = MAX(0, (player->x - w / 2) - 1);
    region.min.y = MAX(0, (player->y - h / 2) - 1);
    region.max.x = MIN((player->x + w / 2) + 1, map->width - 1);
    region.max.y = MIN((player->y + h / 2) + 1, map->height - 1);

    return region;
}


///
/// See if tile is adjacent to a another tile of given type.
/// - parameter x: The tile's x.
/// - parameter y: The tile's y.
/// - parameter type: The tile type to check for.
/// - parameter num_direction: The directions to check. Either
///   `NUM_CARDINAL_DIRECTIONS` (4) or `NUM_DIRECTIONS` (8).
///
bool TileIsAdjacentTo(const map_t * map,
                      int x,
                      int y,
                      tile_type_t type,
                      int num_directions)
{
    const tile_t * tile = GetTile((map_t *)map, x, y);

    for ( direction_t d = 0; d < num_directions; d++ ) {
        const tile_t * check = GetAdjacentTile((map_t *)map, x, y, d);
        if ( check->type == type ) {
            return true;
        }
    }

    return false;
}




#pragma mark - DISTANCE MAP

typedef struct {
    tile_t * tile;
    int x;
    int y;
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
void UpdateDistanceMap(map_t * map, int x, int y, bool ignore_doors)
{
    queue_size = 0;

    for ( int y = 0; y < map->height; y++ ) {
        for ( int x = 0; x < map->width; x++ ) {
            tile_t * tile = GetTile(map, x, y);

            bool valid;
            if ( tile->type == TILE_DOOR && ignore_doors ) {
                valid = true;
            } else {
                valid = !(tile->flags & FLAG(TILE_BLOCKING)); // Walkable
            }

            if ( valid ) {
                queue_size++;
                tile->distance = -1;
            }
        }
    }

    FreeDistanceMapQueue();
    queue = calloc(queue_size, sizeof(*queue));
    head = 0;
    tail = 0;

    tile_t * tile = GetTile(map, x, y);
    tile->distance = 0;
    qtile_t start = { tile, x, y };
    Put(start);

    while ( head != tail ) {
        qtile_t qtile = Get();
        int distance = qtile.tile->distance;

        for ( int d = 0; d < NUM_DIRECTIONS; d++ ) {
            int x1 = qtile.x + x_deltas[d];
            int y1 = qtile.y + y_deltas[d];

            if ( IsInBounds(map, x1, y1) ) {
                tile_t * edge = GetTile(map, x1, y1);

                bool valid_tile;
                bool not_visited = edge->distance == -1;

                if ( edge->type == TILE_DOOR && ignore_doors ) {
                    valid_tile = not_visited;
                } else {
                    valid_tile = not_visited && !(edge->flags & FLAG(TILE_BLOCKING));
                }

                if ( valid_tile ) {
                    // open and not yet visited
                    edge->distance = distance + 1;
                    qtile_t qtile = { edge, x1, y1 };
                    Put(qtile);
                }
            }
        }
    }
}
