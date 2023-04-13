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


vec2_t GetRenderOffset(const actor_t * player)
{
    SDL_Rect viewport = GetLevelViewport(player->game);
    int half_w = (viewport.w - SCALED(TILE_SIZE)) / 2;
    int half_h = (viewport.h - SCALED(TILE_SIZE)) / 2;

    vec2_t offset = {
        .x = (player->tile.x * SCALED(TILE_SIZE) + player->offset_current.x) - half_w,
        .y = (player->tile.y * SCALED(TILE_SIZE) + player->offset_current.y) - half_h,
    };

    return offset;
}





int CompareActors(const void * a, const void * b) {
    const actor_t * actor1 = (const actor_t *)a;
    const actor_t * actor2 = (const actor_t *)b;

    if (actor1->tile.y != actor2->tile.y) {
        return actor1->tile.y - actor2->tile.y;
    } else {
        return actor1->sprite->draw_priority - actor2->sprite->draw_priority;
    }
}


/// - parameter region: the rectangular area of the map to draw or NULL to draw
/// entire map.
void RenderTilesInRegion(const game_t * game, const box_t * region, int tile_size, vec2_t offset, bool debug)
{
    box_t use;
    if ( region == NULL ) {
        use = (box_t){ 0, 0, game->map.width - 1, game->map.height - 1 };
    } else {
        use = *region;
    }

    tile_coord_t coord;
    for ( coord.y = use.min.y; coord.y <= use.max.y; coord.y++ ) {
        for ( coord.x = use.min.x; coord.x <= use.max.x; coord.x++ ) {
            const tile_t * tile = GetTile((map_t *)&game->map, coord);

            int signature = CalculateWallSignature(&game->map, coord, false);

            int pixel_x = coord.x * tile_size - offset.x;
            int pixel_y = coord.y * tile_size - offset.y;

            RenderTile(tile,
                       game->area,
                       signature,
                       pixel_x,
                       pixel_y,
                       tile_size,
                       debug);

            if ( show_debug_info && TileCoordsEqual(coord, game->mouse_tile) ) {
                SDL_Rect highlight = { pixel_x, pixel_y, tile_size, tile_size };
                V_SetRGB(255, 80, 80);
                V_DrawRect(&highlight);
            }
        }
    }
}


void RenderMap(const game_t * game)
{
    const SDL_Rect viewport = GetLevelViewport(game);
    SDL_RenderSetViewport(renderer, &viewport);
    vec2_t offset = game->camera;
    const actors_t * actors = &game->map.actors;

    //
    // Draw all tiles.
    //

    box_t vis_rect = GetVisibleRegion(game);
    RenderTilesInRegion(game, &vis_rect, SCALED(TILE_SIZE), offset, false);

    RenderParticles(&game->particles, DRAW_SCALE, offset);

    //
    // Draw all actors.
    //

    // Make a list of visible actors.
    actor_t visible_actors[MAX_ACTORS];
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
            visible_actors[num_visible_actors++] = *actor;
        }
    }

    SDL_qsort(visible_actors, num_visible_actors, sizeof(actor_t), CompareActors);

    for ( int i = 0; i < num_visible_actors; i++ ) {
        const actor_t * a = &visible_actors[i];
        int size = SCALED(TILE_SIZE);
        int x = a->tile.x * size + a->offset_current.x - game->camera.x;
        int y = a->tile.y * size + a->offset_current.y - game->camera.y;
        RenderActor(a, x, y, size, false);
    }

    SDL_RenderSetViewport(renderer, NULL);
}


/// Is `t2` visible from `t1`?
bool LineOfSight(map_t * map, tile_coord_t t1, tile_coord_t t2)
{
    int dx = abs(t2.x - t1.x);
    int dy = -abs(t2.y - t1.y);
    int sx = t1.x < t2.x ? 1 : -1;
    int sy = t1.y < t2.y ? 1 : -1;
    int err = dx + dy;
    int e2;

    tile_coord_t current = t1;

    while ( current.x != t2.x || current.y != t2.y ) {
        tile_t * tile = GetTile(map, current);

        if ( tile == NULL ) {
            return false;
        }

        if ( tile->flags.blocking ) {
            return false;
        }

        e2 = 2 * err;

        if ( e2 >= dy ) {
            err += dy;
            current.x += sx;
        }

        if ( e2 <= dx ) {
            err += dx;
            current.y += sy;
        }
    }

    return true;
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
    if ( x0 == x1 && y0 == y1 ) {
        return true;
    } else if ( x0 == x1 ) {
        return VLineIsClear(map, x0, y0, y1);
    } else if ( y0 == y1 ) {
        return HLineIsClear(map, y0, x0, x1);
    } else {
        return
        ( HLineIsClear(map, y0, x0, x1) && VLineIsClear(map, x1, y0, y1) )
        ||
        ( HLineIsClear(map, y1, x0, x1) && VLineIsClear(map, x0, y0, y1) );
    }
}


///
/// Get the rectangular region around the player that is currently visible
/// on screen.
///
box_t GetVisibleRegion(const game_t * game)
{
    SDL_Rect viewport = GetLevelViewport(game);

    // Size in tiles.
    int w = viewport.w / SCALED(TILE_SIZE);
    int h = viewport.h / SCALED(TILE_SIZE);

    printf("viewport size in tiles: %d x %d\n", w, h);

    // Include a padding of 1 so tiles don't disappear when scrolling.

    // Get the camera's tile
    vec2_t camera_tile = { game->camera.x, game->camera.y };
    camera_tile.x /= SCALED(TILE_SIZE);
    camera_tile.y /= SCALED(TILE_SIZE);
    const actor_t * player = GetPlayer(game);
    printf("player tile: %d, %d\n", player->tile.x, player->tile.y);
    printf("camera position: %f, %f\n", game->camera.x, game->camera.y);
    printf("camera tile: %f, %f\n", camera_tile.x, camera_tile.y);
//    int half_w = (viewport.w - SCALED(TILE_SIZE)) / 2;
//    int half_h = (viewport.h - SCALED(TILE_SIZE)) / 2;
//    camera_tile.x = (game->camera.x + half_w) / SCALED(TILE_SIZE);
//    camera_tile.y = (game->camera.y + half_h) / SCALED(TILE_SIZE);

    box_t region;
    region.min.x = (camera_tile.x - w / 2);
    region.min.y = (camera_tile.y - h / 2);
    region.max.x = (camera_tile.x + w / 2);
    region.max.y = (camera_tile.y + h / 2);

    CLAMP(region.min.x, 0, game->map.width - 1);
    CLAMP(region.max.x, 0, game->map.width - 1);
    CLAMP(region.min.y, 0, game->map.height - 1);
    CLAMP(region.max.y, 0, game->map.height - 1);
//    region.min.x = MAX(0, (camera_tile.x - w / 2) - 1);
//    region.min.y = MAX(0, (camera_tile.y - h / 2) - 1);
//    region.max.x = MIN((camera_tile.x + w / 2) + 1, game->map.width - 1);
//    region.max.y = MIN((camera_tile.y + h / 2) + 1, game->map.height - 1);

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
/// - parameter ignore_flags: The tile types to be ignored, as bit flags.
void CalculateDistances(map_t * map, tile_coord_t coord, int ignore_flags)
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
                qtile_t next_qtile = { edge, edge_coord };
                Put(next_qtile);
            }
        }
    }
}
