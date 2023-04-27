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

int CalculateWallSignature(const Map * map, TileCoord coord, bool ignore_reveal)
{
    int signature = 0;

    for ( Direction i = 0; i < NUM_DIRECTIONS; i++ ) {
        Tile * adjacent = GetAdjacentTile((Map *)map, coord, i);

        if ( adjacent ) {
            bool is_floor = true;

            if ( adjacent->flags.blocks_movement ) {
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


bool IsInBounds(const Map * map, int x, int y)
{
    return x >= 0 && y >= 0 && x < map->width && y < map->height;
}


Tile * GetAdjacentTile(Map * map, TileCoord coord, Direction direction)
{
    if ( direction == NO_DIRECTION ) {
        return GetTile(map, coord);
    }

    return GetTile(map, AdjacentTileCoord(coord, direction));
}


Tile * GetTileNonConst(Map * map, TileCoord coord)
{
    if ( !IsInBounds(map, coord.x, coord.y) ) {
        return NULL;
    }

    return &map->tiles[coord.y * map->width + coord.x];
}


const Tile * GetTileConst(const Map * map, TileCoord coord)
{
    if ( !IsInBounds(map, coord.x, coord.y) ) {
        return NULL;
    }

    return &map->tiles[coord.y * map->width + coord.x];
}


TileCoord GetCoordinate(const Map * map, int index)
{
    TileCoord coord = { index % map->width, index / map->width };
    return coord;
}


/// - parameter region: the rectangular area of the map to draw or NULL to draw
/// entire map.
void RenderTiles(const World * world, const Box * region, vec2_t offset, bool debug)
{
    const Map * map = &world->map;

    int tile_size;
    if ( debug ) {
        tile_size = area_info[world->area].debug_map_tile_size;
    } else {
        tile_size = SCALED(TILE_SIZE);
    }

    Box use;
    if ( region == NULL ) {
        use = (Box){ 0, 0, map->width - 1, map->height - 1 };
    } else {
        use = *region;
    }

    TileCoord coord;
    for ( coord.y = use.top; coord.y <= use.bottom; coord.y++ ) {
        for ( coord.x = use.left; coord.x <= use.right; coord.x++ ) {
            const Tile * tile = GetTile((Map *)map, coord);

            int signature = CalculateWallSignature(map, coord, false);

            int pixel_x = coord.x * tile_size - offset.x;
            int pixel_y = coord.y * tile_size - offset.y;

            RenderTile(tile,
                       world->area,
                       signature,
                       pixel_x,
                       pixel_y,
                       tile_size,
                       debug);

            if ( show_debug_info && TileCoordsEqual(coord, world->mouse_tile) ) {
                SDL_Rect highlight = { pixel_x, pixel_y, tile_size, tile_size };
                V_SetRGB(255, 80, 80);
                V_DrawRect(&highlight);
            }
        }
    }
}


/// Is `t2` visible from `t1`?
bool LineOfSight(Map * map, TileCoord t1, TileCoord t2)
{
    int dx = abs(t2.x - t1.x);
    int dy = -abs(t2.y - t1.y);
    int sx = t1.x < t2.x ? 1 : -1;
    int sy = t1.y < t2.y ? 1 : -1;
    int err = dx + dy;
    int e2;

    TileCoord current = t1;

    while ( current.x != t2.x || current.y != t2.y ) {
        Tile * tile = GetTile(map, current);

        if ( tile == NULL ) {
            return false;
        }

        if ( tile->flags.blocks_sight ) {
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

static bool HLineIsClear(Map * map, int y, int x0, int x1)
{
    int x = x0;

    // Walk along the x axis.
    while ( x != x1 ) {
        Tile * tile = GetTile(map, ((TileCoord){ x, y }));
        
        if ( tile == NULL ) {
            return false;
        }

        if ( tile->flags.blocks_sight ) {
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

static bool VLineIsClear(Map * map, int x, int y0, int y1)
{
    int y = y0;

    while ( y != y1 ) {
        Tile * tile = GetTile(map, ((TileCoord){ x, y }));
        if ( tile->flags.blocks_sight ) {
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
bool ManhattenPathsAreClear(Map * map, int x0, int y0, int x1, int y1)
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
Box GetVisibleRegion(const Map * map, const RenderInfo * render_info)
{
    SDL_Rect viewport = GetLevelViewport(render_info);

    // Size in tiles.
    int w = viewport.w / SCALED(TILE_SIZE);
    int h = viewport.h / SCALED(TILE_SIZE);

    // Get the camera's tile
    vec2_t camera_tile = render_info->camera;
    camera_tile.x /= SCALED(TILE_SIZE);
    camera_tile.y /= SCALED(TILE_SIZE);

    Box region;
    region.left = MAX(0, (camera_tile.x - w / 2) - 1);
    region.top = MAX(0, (camera_tile.y - h / 2) - 1);

    // Include a padding of +1 so tiles don't disappear when scrolling.
    region.right = MIN((camera_tile.x + w / 2 + 1), map->width - 1);
    region.bottom = MIN((camera_tile.y + h / 2 + 1), map->height - 1);

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
bool TileIsAdjacentTo(const Map * map,
                      TileCoord coord,
                      TileType type,
                      int num_directions)
{
    for ( Direction d = 0; d < num_directions; d++ ) {
        const Tile * check = GetAdjacentTile((Map *)map, coord, d);
        if ( check->type == type ) {
            return true;
        }
    }

    return false;
}

// TODO: dungeon tiles not resetting
void ResetTileVisibility(Map * map,
                         TileCoord player_tile,
                         const RenderInfo * render_info)
{
    Box vis = GetVisibleRegion(map, render_info);

    TileCoord coord;
    for ( coord.y = vis.top; coord.y <= vis.bottom; coord.y++ ) {
        for ( coord.x = vis.left; coord.x <= vis.right; coord.x++ ) {
//            if ( LineOfSight(map, player_tile, coord) ) {
            Tile * tile = GetTile(map, coord);
            tile->flags.visible = false;
//            }
        }
    }
}


#pragma mark - DISTANCE MAP

typedef struct {
    Tile * tile;
    TileCoord coord;
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
void CalculateDistances(Map * map, TileCoord coord, int ignore_flags)
{
    queue_size = 0;

    TileCoord c;
    for ( c.y = 0; c.y < map->height; c.y++ ) {
        for ( c.x = 0; c.x < map->width; c.x++ ) {
            Tile * tile = GetTile(map, c);

            // TODO: the queue was occasionally the wrong size (too small)
//            if ( (ignore_flags & FLAG(tile->type)) || !tile->flags.blocks_movement ) {
//                queue_size++;
                tile->distance = -1;
//            }
        }
    }

    queue_size = map->width * map->height;

    FreeDistanceMapQueue();
    queue = calloc(queue_size, sizeof(*queue));
    head = 0;
    tail = 0;

    Tile * tile = GetTile(map, coord);
    tile->distance = 0;
    qtile_t start = { tile, coord };
    Put(start);

    while ( head != tail ) {
        qtile_t qtile = Get();
        int distance = qtile.tile->distance;

        for ( int d = 0; d < NUM_DIRECTIONS; d++ ) {
            Tile * edge = GetAdjacentTile(map, qtile.coord, d);
            TileCoord edge_coord = AdjacentTileCoord(qtile.coord, d);

            if ( edge // inbounds
                && edge->distance == -1 // not yet visited
                && ( (ignore_flags & FLAG(tile->type)) || !tile->flags.blocks_movement ) )
            {
                // open and not yet visited
                edge->distance = distance + 1;
                qtile_t next_qtile = { edge, edge_coord };
                Put(next_qtile);
            }
        }
    }
}
