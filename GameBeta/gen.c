//
//  gen.c
//  GameBeta
//
//  Created by Thomas Foster on 11/4/22.
//

#include "main.h"
#include "mathlib.h"

typedef char ids_t[MAP_HEIGHT][MAP_WIDTH];

// TODO: this could be better: try another direction if blocked.
/// Carve out a hallway recursively, setting each new floor tile with the
/// current region ID.
static void GenerateHallway_r(tiles_t map,
                              ids_t ids,
                              int current_id,
                              int x,
                              int y)
{
    map[y][x].type = TILE_FLOOR; // Open up this spot
    ids[y][x] = current_id;

    int delay = 25;
    if ( show_map_gen ) {
        DebugRenderTiles(map);
        SDL_Delay(delay);
    }

    struct {
        SDL_Point coord;
        int direction;
    } directions[4];

    int num_open_directions = 0;

    // Make a list of the coordinates of possible directions.
    for ( int i = 0; i < 4; i++ ) {
        int next_x = x + x_dirs[i] * 2;
        int next_y = y + y_dirs[i] * 2;

        if ( IsInBounds(next_x, next_y)
            && map[next_y][next_x].type == TILE_WALL )
        {
            directions[num_open_directions].coord.x = next_x;
            directions[num_open_directions].coord.y = next_y;
            directions[num_open_directions].direction = i;
            num_open_directions++;
        }
    }

    if ( num_open_directions == 0 ) {
        return;
    }

    // Pick a random direction and open it up.
    int i = Random(0, num_open_directions - 1);
    int dir = directions[i].direction;
    SDL_Point next = directions[i].coord;

    // Clear the spot in between this and the next.
    int inbetween_x = next.x - x_dirs[dir];
    int inbetween_y = next.y - y_dirs[dir];
    map[inbetween_y][inbetween_x].type = TILE_FLOOR;
    ids[inbetween_y][inbetween_x] = current_id;

    if ( show_map_gen ) {
        CheckForShowMapGenCancel();
        DebugRenderTiles(map);
        printf("place a hallway tile with id %d\n", current_id);
        SDL_Delay(delay);
    }

    GenerateHallway_r(map, ids, current_id, next.x, next.y);
}

/// Change all `from` IDs to `to`.
void ChangeAllIDs(ids_t ids, int from, int to)
{
    for ( int y = 0; y < MAP_HEIGHT; y++ ) {
        for ( int x = 0; x < MAP_WIDTH; x++ ) {
            if ( ids[y][x] == from ) {
                ids[y][x] = to;
            }
        }
    }
}

// A map tile that is adjacent to the main region and other different region.
typedef struct {
    int x, y;
    int region; // the other region
    bool valid;
} connector_t;

#define MAP_EDGE_ID (-2)
#define MAP_WALL_ID (-1)

int GetConnectors(ids_t ids, int main_region, connector_t * out)
{
    int num_connectors = 0;

    for ( int y = 1; y < MAP_HEIGHT - 2; y++ ) {
        for ( int x = 1; x < MAP_WIDTH - 2; x++ ) {

            // Potential connectors must be wall tiles.
            if ( ids[y][x] != MAP_WALL_ID ) {
                continue;
            }

            // Check all four adjacent tiles to see if (x, y) touches the main
            // region and another region.
            bool touches_main = false;
            int other_region = -1;

            for ( int d = 0; d < 4; d++ ) {
                int x1 = x + x_dirs[d];
                int y1 = y + y_dirs[d];

                if ( ids[y1][x1] == main_region ) {
                    touches_main = true;
                }

                if ( ids[y1][x1] >= 0 && ids[y1][x1] != main_region ) {
                    other_region = ids[y1][x1];
                }
            }

            if ( touches_main && other_region != -1 ) {
                // It's valid, add to list.
                out[num_connectors].x = x;
                out[num_connectors].y = y;
                out[num_connectors].region = other_region;
                num_connectors++;
            }
        }
    }

    return num_connectors;
}

/// Get an array of coordinates of tiles that are dead ends.
int GetDeadEnds(tiles_t map, SDL_Point * out)
{
    int count = 0;

    for ( int y = 0; y < MAP_HEIGHT; y++ ) {
        for ( int x = 0; x < MAP_WIDTH; x++ ) {
            if ( map[y][x].type != TILE_FLOOR ) {
                continue;
            }

            // Count the number of connections to this tile.
            int connection_count = 0;
            for ( int d = 0; d < 4; d++ ) {
                if ( map[y + y_dirs[d]][x + x_dirs[d]].type == TILE_FLOOR ) {
                    connection_count++;
                }
            }

            if ( connection_count == 1 ) {
                // It's a dead end.
                out[count].x = x;
                out[count].y = y;
                count++;
            }
        }
    }

    return count;
}

static void GetRoomCorners(SDL_Rect room, SDL_Point corners[4])
{
    int top = room.y;
    int left = room.x;
    int right = room.x + room.w - 1;
    int bottom = room.y + room.h - 1;

    corners[0] = (SDL_Point){ left,  top };
    corners[1] = (SDL_Point){ right, top };
    corners[2] = (SDL_Point){ left,  bottom };
    corners[3] = (SDL_Point){ right, bottom };
}

// https://www.tomstephensondeveloper.co.uk/post/creating-simple-procedural-dungeon-generation
void GenerateMap(game_t * game)
{
    map_t * map = &game->map;

    //
    // Init ids and tiles.
    //
    // IDs are unique identifers for each 'region'. Regions are
    // any unconnected spaces. Unset tiles start with a negative ID indicating
    // it's still a floor, or a floor on the edge of the map.
    ids_t ids;

    for ( int y = 0; y < MAP_HEIGHT; y++ ) {
        for ( int x = 0; x < MAP_WIDTH; x++ ) {
            if ( x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1 ) {
                ids[y][x] = MAP_EDGE_ID;
            } else {
                ids[y][x] = MAP_WALL_ID;
            }

            memset(&map->tiles[y][x], 0, sizeof(map->tiles[y][x]));
            map->tiles[y][x].type = TILE_WALL;
            //map->tiles[y][x].flags = 0;
            map->tiles[y][x].variety = Random(0, 255);
            //map->tiles[y][x].visible = false;
            //map->tiles[y][x].revealed = false;
            //map->tiles[y][x].light = 0;
            //map->tiles[y][x].light_target = 0;
        }
    }

    map->num_rooms = 0;
    int current_id = 0; // Regions

    //
    // Spawn rooms
    //

    for ( int tries = 1; tries <= 50; tries++ ) {
        int size = Random(1, 3) * 2 + 1; // 3 - 7
        int rectangularity = Random(0, 1 + size / 2) * 2; // 0 - 8

        SDL_Rect rect;
        rect.w = size;
        rect.h = size;
        if ( Chance( 1.0f / 2.0f ) ) {
            rect.w += rectangularity;
        } else {
            rect.h += rectangularity;
        }

        rect.x = Random(1, (MAP_WIDTH - rect.w) / 2) * 2 - 1;
        rect.y = Random(1, (MAP_HEIGHT - rect.h) / 2) * 2 - 1;

        // Check that this potential room does not overlap with any
        // existing rooms.
        for ( int room = 0; room < map->num_rooms; room++ ) {
            if ( SDL_HasIntersection(&rect, &map->rooms[room]) ) {
                goto next_try;
            }
        }

        // Spot is clear. Open up the room and set its floor tiles to the
        // current region ID.
        map->rooms[map->num_rooms++] = rect;
        for ( int y = rect.y; y < rect.y + rect.h; y++ ) {
            for ( int x = rect.x; x < rect.x + rect.w; x++ ) {
                map->tiles[y][x].type = TILE_FLOOR;
                ids[y][x] = current_id;
            }
        }
        printf("placed a room with id %d\n", current_id);
        current_id++;

        if ( show_map_gen ) {
            CheckForShowMapGenCancel();
            DebugRenderTiles(game->map.tiles);
            SDL_Delay(200);
        }

    next_try:
        ;
    }

    //
    // Generate hallways.
    //

    int num_potentials;
    do {
        num_potentials = 0;
        SDL_Point potentials[MAP_WIDTH * MAP_HEIGHT];

        // Check all odd positions and make a list of viable spots at
        // which to begin a hallway.
        for ( int y = 1; y <= MAP_HEIGHT - 2; y += 2 ) {
            for ( int x = 1; x <= MAP_WIDTH - 2; x += 2 ) {
                if ( map->tiles[y][x].type == TILE_WALL ) {
                    potentials[num_potentials++] = (SDL_Point){ x, y };
                }
            }
        }

        if ( num_potentials > 0 ) {
            // There are still spots at which to start a hallway. Select
            // a random one and begin carving it out.
            int i = Random(0, num_potentials - 1);
            GenerateHallway_r(map->tiles,
                              ids,
                              current_id,
                              potentials[i].x,
                              potentials[i].y);
            current_id++;
        }
    } while ( num_potentials > 0 );

    const int num_regions = current_id;

    //
    // Open up doors between rooms and hallways.
    //

    // Pick a random region to start.
    int main_region = Random(0, num_regions - 1);

    // While there are still connectors adjacent to the main region,
    int num_connectors = 0;
    connector_t connectors[MAP_WIDTH * MAP_HEIGHT];
    while ( (num_connectors = GetConnectors(ids, main_region, connectors)) ) {
        // Pick a random connector, open it up, and merge the other region with
        // the main.
        connector_t connector = connectors[Random(0, num_connectors - 1)];
        map->tiles[connector.y][connector.x].type = TILE_FLOOR;
        map->tiles[connector.y][connector.x].flags |= TILE_FLAG_DOOR;
        ChangeAllIDs(ids, connector.region, main_region);

        if ( show_map_gen ) {
            CheckForShowMapGenCancel();
            DebugRenderTiles(game->map.tiles);
            SDL_Delay(25);
        }
    }

    //
    // Eliminate deadends. (Nobody likes backtracking)
    //

    int num_deadends = 0;
    SDL_Point deadends[MAP_WIDTH * MAP_HEIGHT];
    while ( (num_deadends = GetDeadEnds(map->tiles, deadends)) ) {
        for ( int i = 0; i < num_deadends; i++ ) {
            map->tiles[deadends[i].y][deadends[i].x].type = TILE_WALL;
            if ( show_map_gen ) {
                CheckForShowMapGenCancel();
                DebugRenderTiles(game->map.tiles);
                SDL_Delay(25);
            }
        }
    }

    //
    // Spawn actors.
    //

    game->num_actors = 0;

    // Spawn player.
    SDL_Rect room = map->rooms[0];
    int x = Random(room.x, room.x + room.w - 1);
    int y = Random(room.y, room.y + room.h - 1);
    SpawnActor(game, ACTOR_PLAYER, x, y);

    for ( int i = 0; i < map->num_rooms; i++ ) {

#if 0
        if ( Chance(1.0f / 4.0f) ) {
            // Spawn some torches. TODO: for testing purposes only
            SDL_Point corners[4];
            GetRoomCorners(map->rooms[i], corners);
            for ( int j = 0; j < 4; j++ ) {
                SpawnActor(game, ACTOR_TORCH, corners[j].x, corners[j].y);
            }
        }
#endif

        // TODO: Testing
        int x = map->rooms[i].x + map->rooms[i].w / 2;
        int y = map->rooms[i].y + map->rooms[i].h / 2;
        SpawnActor(game, ACTOR_BLOB, x, y);

        if ( show_map_gen ) {
            DebugWaitForKeyPress();
        }
    }
}
