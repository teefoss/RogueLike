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

const int x_dirs[NUM_DIRECTIONS] = {  0, 0, -1, 1, -1,  1, -1, 1 };
const int y_dirs[NUM_DIRECTIONS] = { -1, 1,  0, 0, -1, -1,  1, 1 };

#define NUM_TILE_SPRITES 15

bool IsInBounds(int x, int y)
{
    return x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_HEIGHT;
}

tile_t * GetAdjacentTile(tiles_t tiles, int x, int y, direction_t direction)
{
    return &tiles[y + y_dirs[direction]][x + x_dirs[direction]];
}

void RenderTile(const tile_t * tile, int x, int y, int scale, bool do_light)
{
    SDL_Texture * tiles = GetTexture("assets/tiles.png");

    SDL_Rect src, dst;
    src.w = src.h = TILE_SIZE;
    dst.w = dst.h = scale;
    dst.x = x;
    dst.y = y;

    if ( do_light ) {
        SDL_SetTextureColorMod(tiles, tile->light, tile->light, tile->light);
    } else {
        SDL_SetTextureColorMod(tiles, 255, 255, 255);
    }

    switch ( tile->type ) {
        case TILE_WALL:
            src.x = (tile->variety % 4) * TILE_SIZE;
            src.y = TILE_SIZE * 2;
            break;
        case TILE_FLOOR:
            if ( tile->variety < 112 ) {
                src.x = TILE_SIZE * (tile->variety % 8);
            } else {
                src.x = 0;
            }
            src.y = TILE_SIZE * 1;
            break;
        default:
            break;
    }

    V_DrawTexture(tiles, &src, &dst);

    if ( tile->type == TILE_FLOOR ) {
        const u8 * keys = SDL_GetKeyboardState(NULL);
        if ( keys[SDL_SCANCODE_F1] ) {
            V_SetGray(255);
            V_PrintString(dst.x, dst.y, "%d", tile->distance);
        }
    }
}

void RenderMap(const game_t * game)
{
    // Calculate the draw offset.
    int offset_x = 0;
    int offset_y = 0;
    for ( int i = 0; i < game->num_actors; i++ ) {
        const actor_t * a = &game->actors[i];
        if ( a->type == ACTOR_PLAYER ) {
            int half_w = (GAME_WIDTH - RENDER_TILE_SIZE) / 2;
            int half_h = (GAME_HEIGHT - RENDER_TILE_SIZE) / 2;
            offset_x = (a->x * RENDER_TILE_SIZE + a->offset.x) - half_w;
            offset_y = (a->y * RENDER_TILE_SIZE + a->offset.y) - half_h;
        }
    }

    //
    // Draw all tiles.
    //

    box_t visible_region = GetVisibleRegion(&game->actors[0]);

    for ( int y = visible_region.min.y; y <= visible_region.max.y; y++ ) {
        for ( int x = visible_region.min.x; x <= visible_region.max.x; x++ ) {
            const tile_t * tile = &game->map.tiles[y][x];

            int pixel_x = (x * RENDER_TILE_SIZE) - offset_x;
            int pixel_y = (y * RENDER_TILE_SIZE) - offset_y;
            RenderTile(tile, pixel_x, pixel_y, RENDER_TILE_SIZE, true);
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

        if ( game->map.tiles[actor->y][actor->x].visible
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
        RenderActor(visible_actors[i], offset_x, offset_y);
    }
}

void DebugRenderTiles(tiles_t tiles)
{
    V_ClearRGB(0, 0, 0);

    for ( int y = 0; y < MAP_HEIGHT; y++ ) {
        for ( int x = 0; x < MAP_WIDTH; x++ ) {
            RenderTile(&tiles[y][x], x * 8, y * 8, 8, false);
        }
    }

    V_Refresh();
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
        tile_t * tile = &game->map.tiles[y1][x1];
//        MapNode * node = MapNodeAt(pt0);

        if ( reveal ) {
            tile->visible = true;
            tile->revealed = true;

            // For floor tiles, also reveal any surrounding wall tiles.
            if ( tile->type == TILE_FLOOR ) {
                for ( int d = 0; d < NUM_DIRECTIONS; d++ ) {
                    tile_t * adjacent = GetAdjacentTile(game->map.tiles, x1, y1, d);
                    if ( adjacent->type == TILE_WALL ) {
                        adjacent->visible = true;
                        adjacent->revealed = true;
                    }
                }
            }
        }

        if ( x1 == x2 && y1 == y2 ) {
            return true;
        }

        if ( tile->type == TILE_WALL ) {
            return false;
        }

        actor_t * actor = game->actors;
        for ( int i = 0; i < game->num_actors; i++, actor++ ) {
            if ( actor->flags & ACTOR_FLAG_BLOCKS_SIGHT
                && (actor->x == x1 && actor->y == y1) )
            {
                return false;
            }
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

box_t GetVisibleRegion(const actor_t * player)
{
    int w = GAME_WIDTH / RENDER_TILE_SIZE;
    int h = GAME_HEIGHT / RENDER_TILE_SIZE;

    // Include a padding of 1 so tiles don't disappear when scrolling.

    box_t region;
    region.min.x = MAX(0, (player->x - w / 2) - 1);
    region.min.y = MAX(0, (player->y - h / 2) - 1);
    region.max.x = MIN((player->x + w / 2) + 1, MAP_WIDTH - 1);
    region.max.y = MIN((player->y + h / 2) + 1, MAP_HEIGHT - 1);

    return region;
}

//
// UpdateDistanceMap and friends
//

typedef struct {
    tile_t * tile;
    int x;
    int y;
} qtile_t;

static qtile_t queue[MAP_WIDTH * MAP_HEIGHT];
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

/// For all walkable tiles, update tile `distance` property
/// with distance to x, y.
void UpdateDistanceMap(tiles_t tiles, int x, int y)
{
    queue_size = 0;
    for ( int y = 0; y < MAP_HEIGHT; y++ ) {
        for ( int x = 0; x < MAP_WIDTH; x++ ) {
            if ( tiles[y][x].type == TILE_FLOOR ) {
                queue_size++;
                tiles[y][x].distance = -1;
            }
        }
    }

    head = 0;
    tail = 0;

    tiles[y][x].distance = 0;
    qtile_t start = { &tiles[y][x], x, y };
    Put(start);

    while ( head != tail ) {
        qtile_t qtile = Get();
        int distance = qtile.tile->distance;

        for ( int d = 0; d < NUM_DIRECTIONS; d++ ) {
            int x1 = qtile.x + x_dirs[d];
            int y1 = qtile.y + y_dirs[d];

            if ( IsInBounds(x1, y1) ) {
                tile_t * edge = &tiles[y1][x1];

                if ( edge->distance == -1 && edge->type == TILE_FLOOR ) {
                    // valid tile, not yet visited
                    edge->distance = distance + 1;
                    qtile_t qtile = { edge, x1, y1 };
                    Put(qtile);
                }
            }
        }
    }
}
