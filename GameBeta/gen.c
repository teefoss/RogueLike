//
//  gen.c
//  GameBeta
//
//  Created by Thomas Foster on 11/4/22.
//

#include "main.h"
#include "mathlib.h"
#include "texture.h"
#include "video.h"

const int debug_tile_size = 16;

tile_id_t * TileID(map_t * map, int x, int y)
{
    return &map->tile_ids[y * map->width + x];
}


void RenderTilesWithDelay(map_t * map)
{
    if ( show_map_gen ) {
        CheckForShowMapGenCancel();
        V_ClearRGB(0, 0, 0);
        DebugRenderTiles(map);
        V_Refresh();
        SDL_Delay(25);
    }
}


void DebugRenderTiles(map_t * map)
{
    SDL_Texture * tiles = GetTexture("assets/tiles2.png");

    SDL_Rect src = { .w = TILE_SIZE, .h = TILE_SIZE };
    SDL_Rect dst = { .w = debug_tile_size, .h = debug_tile_size };

    for ( int y = 0; y < map->height; y++ ) {
        for ( int x = 0; x < map->width; x++ ) {
            tile_t * tile = GetTile(map, x, y);
            if ( tile->type == TILE_WALL ) {
                src.x = 0;
                src.y = 0;
            } else {
                src.x = tile->sprite_cell.x * TILE_SIZE;
                src.y = tile->sprite_cell.y * TILE_SIZE;
                if ( tile->num_variants ) {
                    src.x += (tile->variety % tile->num_variants) * TILE_SIZE;
                }
            }

            dst.x = x * debug_tile_size;
            dst.y = y * debug_tile_size;

            V_DrawTexture(tiles, &src, &dst);
        }
    }
}


void DebugRenderActors(game_t * game)
{
    SDL_Texture * actor_sheet = GetTexture("assets/actors.png");

    SDL_Rect src = { .w = TILE_SIZE, .h = TILE_SIZE };
    SDL_Rect dst = { .w = debug_tile_size, .h = debug_tile_size };

    for ( actor_t * a = game->actors; a < game->actors + game->num_actors; a++ ) {
        src.x = a->sprite_cell.x * TILE_SIZE;
        src.y = a->sprite_cell.y * TILE_SIZE;
        dst.x = a->x * debug_tile_size;
        dst.y = a->y * debug_tile_size - 4;

        V_DrawTexture(actor_sheet, &src, &dst);
    }
}


///
/// Carve out a hallway recursively, setting each new floor tile with the
/// current region ID.
///
static void GenerateHallway_r(map_t * map,
                              int current_id,
                              int x,
                              int y)
{
    tile_t * here = GetTile(map, x, y);
    tile_id_t * id = TileID(map, x, y);
    *here = CreateTile(TILE_FLOOR);
    *id = current_id;

    RenderTilesWithDelay(map);

    struct {
        SDL_Point coord;
        int direction;
    } directions[NUM_CARDINAL_DIRECTIONS];

    int num_open_directions = 0;

    // Make a list of the coordinates of possible directions.
    for ( int i = 0; i < NUM_CARDINAL_DIRECTIONS; i++ ) {
        int next_x = x + x_deltas[i] * 2;
        int next_y = y + y_deltas[i] * 2;
        tile_t * next = GetTile(map, next_x, next_y);

        if ( IsInBounds(map, next_x, next_y) && next->type == TILE_WALL ) {
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
    int inbetween_x = next.x - x_deltas[dir];
    int inbetween_y = next.y - y_deltas[dir];
//    map[inbetween_y][inbetween_x] = CreateTile(TILE_FLOOR);
//    ids[inbetween_y][inbetween_x] = current_id;
    tile_t * inbetween = GetTile(map, inbetween_x, inbetween_y);
    *inbetween = CreateTile(TILE_FLOOR);
    tile_id_t * inbetween_id = TileID(map, inbetween_x, inbetween_y);
    *inbetween_id = current_id;

    RenderTilesWithDelay(map);

    GenerateHallway_r(map, current_id, next.x, next.y);
}




/// Change all `from` IDs to `to`.
void ChangeAllIDs(map_t * map, tile_id_t from, tile_id_t to)
{
    for ( int i = 0; i < map->width * map->height; i++ ) {
        if ( map->tile_ids[i] == from ) {
            map->tile_ids[i] = to;
        }
    }
}




// A map tile that is adjacent to the main region and another different region.
typedef struct {
    int x, y;
    int region; // the other region
    bool valid;
} connector_t;

#define MAP_EDGE_ID (-2)
#define MAP_WALL_ID (-1)

int GetConnectors(map_t * map, int main_region, connector_t * out)
{
    int num_connectors = 0;

    for ( int y = 1; y < MAP_HEIGHT - 2; y++ ) {
        for ( int x = 1; x < MAP_WIDTH - 2; x++ ) {

            // Potential connectors must be wall tiles.
            if ( *TileID(map, x, y) != MAP_WALL_ID ) {
                continue;
            }

            // Check all four adjacent tiles to see if (x, y) touches the main
            // region and another region.
            bool touches_main = false;
            int other_region = -1;

            for ( int d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
                int x1 = x + x_deltas[d];
                int y1 = y + y_deltas[d];

                tile_id_t id = *TileID(map, x1, y1);
                if ( id == main_region ) {
                    touches_main = true;
                }

                if ( id >= 0 && id != main_region ) {
                    other_region = id;
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
int GetDeadEnds(map_t * map, SDL_Point * out)
{
    int count = 0;

    for ( int y = 1; y < map->height - 1; y++ ) {
        for ( int x = 1; x < map->width - 1; x++ ) {
            tile_t * tile = GetTile(map, x, y);

            if ( tile->type == TILE_FLOOR ) {

                // Count the number of non-wall connections to this tile.
                int connection_count = 0;
                for ( direction_t d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
                    tile_t * adj = GetTile(map, x + x_deltas[d], y + y_deltas[d]);
                    if ( adj->type != TILE_WALL ) {
                        connection_count++;
                    }
                }

                if ( connection_count == 1 ) { // It's a dead end.
                    out[count].x = x;
                    out[count].y = y;
                    count++;
                }
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


static SDL_Point GetRandomPointInRoom(map_t * map, int room_num)
{
    SDL_Rect room = map->rooms[room_num];
    SDL_Point pt;
    pt.x = Random(room.x, room.x + room.w - 1);
    pt.y = Random(room.y, room.y + room.h - 1);

    return pt;
}


static SDL_Point GetRandomRoomCorner(map_t * map, int room_num)
{
    SDL_Rect room = map->rooms[room_num];
    SDL_Point corners[4];
    GetRoomCorners(room, corners);
    return corners[Random(0, 3)];
}

SDL_Point GetRoomCenter(SDL_Rect room)
{
    SDL_Point center = { room.x + room.w / 2, room.y + room.h / 2 };
    return center;
}


///
/// Get a list of tiles in a room that are unoccupied by an actor and not
/// adjacent to a door.
/// - returns: The number of valid tiles.
///
int GetValidRoomTiles(const game_t * game, int room_num, SDL_Point * out)
{
    bool occupied[MAP_HEIGHT][MAP_WIDTH] = { 0 };
    for ( int i = 0; i < game->num_actors; i++ ) {
        occupied[game->actors[i].y][game->actors[i].x] = true;
    }

    SDL_Rect room = game->map.rooms[room_num];
    int num_valid_tiles = 0;

    for ( int y = room.y; y < room.y + room.h; y++ ) {
        for ( int x = room.x; x < room.x + room.w; x++ ) {

            bool valid = true;

            if ( occupied[y][x] ) {
                valid = false;
            }

            for ( direction_t d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
                const tile_t * t = GetAdjacentTile((map_t *)&game->map,
                                                   x,
                                                   y,
                                                   d);
                if ( t->type == TILE_DOOR || t->type == TILE_GOLD_DOOR ) {
                    valid = false;
                    break;
                }
            }

            if ( valid ) {
                out[num_valid_tiles++] = (SDL_Point){ x, y };
            }
        }
    }

    return num_valid_tiles;
}


void SpawnPlayerAndStartTile(game_t * game)
{
    SDL_Point pt = GetRoomCenter(game->map.rooms[0]);
    printf("player start: %d, %d\n", pt.x, pt.y);

    SpawnActor(game, ACTOR_PLAYER, pt.x, pt.y);
    *GetTile(&game->map, pt.x, pt.y) = CreateTile(TILE_START);
}


void SpawnGoldKey(game_t * game, SDL_Point * points)
{
    // Select one of the rooms other than the start and end rooms.
    int room_num = Random(1, game->map.num_rooms - 2);
    int num_valid_tiles = GetValidRoomTiles(game, room_num, points);

    int i = Random(0, num_valid_tiles - 1);
    SpawnActor(game, ACTOR_GOLD_KEY, points[i].x, points[i].y);
}


///
/// Try to place the exit in the corner of a room, choosing a corner that
/// is not adjacent to a door. If none are found, place the exit in the center
/// of the room.
///
void SpawnExit(map_t * map)
{
    int exit_room = map->num_rooms - 1;

    SDL_Point corners[4];
    GetRoomCorners(map->rooms[exit_room], corners);

    // Make a list of viable corners.
    int num_usable = 0;
    int usable_indices[4] = { 0 };

    for ( int i = 0; i < 4; i++ ) {
        if ( !TileIsAdjacentTo(map,
                               corners[i].x,
                               corners[i].y,
                               TILE_DOOR,
                               NUM_CARDINAL_DIRECTIONS) )
        {
            usable_indices[num_usable++] = i;
        }
    }

    SDL_Point pt;

    if ( num_usable == 0 ) {
        pt = GetRoomCenter(map->rooms[exit_room]);
    } else {
        pt = corners[usable_indices[Random(0, num_usable - 1)]];
    }

    *GetTile(map, pt.x, pt.y) = CreateTile(TILE_EXIT);

    // Turn any doors adjacent to the exit room into gold doors.
    SDL_Rect rect = map->rooms[exit_room];

    // Expand by one to include any doors touching the exit room.
    // TODO: do not turn doors 'past' exit room
    rect.x--;
    rect.y--;
    rect.w++;
    rect.h++;

    // TODO: do this not dumb.
    for ( int y = rect.y; y <= rect.y + rect.h; y++ ) {
        for ( int x = rect.x; x <= rect.x + rect.w; x++ ) {
            tile_t * tile = GetTile(map, x, y);
            if ( tile->type == TILE_DOOR ) {
                *tile = CreateTile(TILE_GOLD_DOOR);
            }
        }
    }
}


// https://www.tomstephensondeveloper.co.uk/post/creating-simple-procedural-dungeon-generation
void GenerateDungeon(game_t * game, int width, int height)
{
    if ( width % 2 == 0 || height % 2 == 0 ) {
        Error("Dugeon width and height must be odd");
    }

    map_t * map = &game->map;
    map->width = width;
    map->height = height;

    //
    // Init tiles.
    //

    // IDs are unique identifers for each 'region'. Regions are
    // any unconnected spaces. Unset tiles start with a negative ID indicating
    // it's still a floor, or a floor on the edge of the map.

    int map_size = width * height;

    if ( map->tiles ) {
        free(map->tiles);
    }

    map->tiles = calloc(map_size, sizeof(*map->tiles));
    if ( map->tiles == NULL ) {
        Error("Could not allocate map tiles array");
    }

    if ( map->tile_ids ) {
        free(map->tile_ids);
    }

    map->tile_ids = calloc(map_size, sizeof(*map->tile_ids));
    if ( map->tile_ids == NULL ) {
        Error("Could not allocate map tile id array");
    }

    for ( int y = 0; y < height; y++ ) {
        for ( int x = 0; x < width; x++ ) {
            tile_t * t = GetTile(map, x, y);
            tile_id_t * id = TileID(map, x, y);

            *t = CreateTile(TILE_WALL);

            if ( x == 0 || x == width - 1 || y == 0 || y == height - 1 ) {
                *id = MAP_EDGE_ID;
            } else {
                *id = MAP_WALL_ID;
            }
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
        if ( Chance(0.5f) ) {
            rect.w += rectangularity;
        } else {
            rect.h += rectangularity;
        }

        rect.x = Random(1, (width - rect.w) / 2) * 2 - 1;
        rect.y = Random(1, (height - rect.h) / 2) * 2 - 1;

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
                tile_t * tile = GetTile(map, x, y);
                *tile = CreateTile(TILE_FLOOR);
                tile->flags |= FLAG(TILE_ROOM);
                *TileID(map, x, y) = current_id;
            }
        }
        printf("placed a room with id %d (%d, %d, %d, %d)\n",
               current_id,
               rect.x, rect.y, rect.w, rect.h);
        current_id++;

        RenderTilesWithDelay(map);

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
        for ( int y = 1; y <= map->height - 2; y += 2 ) {
            for ( int x = 1; x <= map->width - 2; x += 2 ) {
                if ( GetTile(map, x, y)->type == TILE_WALL ) {
//                if ( map->tiles[y][x].type == TILE_WALL ) {
                    potentials[num_potentials++] = (SDL_Point){ x, y };
                }
            }
        }

        if ( num_potentials > 0 ) {
            // There are still spots at which to start a hallway. Select
            // a random one and begin carving it out.
            int i = Random(0, num_potentials - 1);
            GenerateHallway_r(map,
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

    SDL_Point potential_door_locations[MAP_WIDTH * MAP_HEIGHT];
    int num_potential_door_locations = 0;

    // Pick a random region to start.
    int main_region = Random(0, num_regions - 1);

    // While there are still connectors adjacent to the main region,
    int num_connectors = 0;
    connector_t connectors[MAP_WIDTH * MAP_HEIGHT];
    while ( (num_connectors = GetConnectors(map, main_region, connectors)) ) {

        // Pick a random connector, open it up, and merge the other region with
        // the main.
        connector_t connector = connectors[Random(0, num_connectors - 1)];
        *GetTile(map, connector.x, connector.y) = CreateTile(TILE_FLOOR);
        potential_door_locations[num_potential_door_locations++] = (SDL_Point){
            connector.x,
            connector.y
        };
        ChangeAllIDs(map, connector.region, main_region);

        RenderTilesWithDelay(map);
    }

    //
    // Eliminate deadends.
    //

    int num_deadends = 0;
    SDL_Point * deadends = calloc(map_size, sizeof(*deadends));

    while ( (num_deadends = GetDeadEnds(map, deadends)) ) {
        for ( int i = 0; i < num_deadends; i++ ) {
            *GetTile(map, deadends[i].x, deadends[i].y) = CreateTile(TILE_WALL);
            RenderTilesWithDelay(map);
        }
    }

    free(deadends);

    //
    // Add doors
    //

    for ( int i = 0; i < num_potential_door_locations; i++ ) {
        SDL_Point p = potential_door_locations[i];

        tile_t * tile = GetTile(map, p.x, p.y);
        if ( tile->type == TILE_FLOOR ) {
            *tile = CreateTile(TILE_DOOR);

            RenderTilesWithDelay(map);
        }
    }

    //
    // Remove invalid doors.
    //

    for ( int y = 0; y < height; y++ ) {
        for ( int x = 0; x < width; x++ ) {
            if ( GetTile(map, x, y)->type == TILE_DOOR ) {

                tile_t * adjacents[NUM_CARDINAL_DIRECTIONS];
                for ( int d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
                    adjacents[d] = GetAdjacentTile(map, x, y, d);
                }

                // Deadend-clearing may have rendered some door spots invalid.
                // Check if still valid.
                if ((adjacents[NORTH]->type == TILE_FLOOR
                    && adjacents[SOUTH]->type == TILE_FLOOR)
                    || (adjacents[WEST]->type == TILE_FLOOR &&
                        adjacents[EAST]->type == TILE_FLOOR))
                {

                } else {
                    // This spot is no longer a valid door.
                    *GetTile(map, x, y) = CreateTile(TILE_WALL);

                    RenderTilesWithDelay(map);
                }
            }
        }
    }

    SpawnExit(map);

    //
    // Spawn actors.
    //

    SDL_Point * valid_points = calloc(map_size, sizeof(*valid_points));
    game->num_actors = 0;
    SpawnPlayerAndStartTile(game);
    SpawnGoldKey(game, valid_points);

    // In each room...
    for ( int i = 0; i < map->num_rooms; i++ ) {
        float chance = 1.0f;
        int area = map->rooms[i].w * map->rooms[i].h;
        int max_monsters = area / 9;

        for ( int j = 0; j < 4; j++ ) {
            if ( Chance(chance) ) {
                int num_points = GetValidRoomTiles(game, i, valid_points);
                int point_index = Random(0, num_points - 1);
                SpawnActor(game,
                           ACTOR_BLOB,
                           valid_points[point_index].x,
                           valid_points[point_index].y);
                --max_monsters;
            }

            if ( max_monsters == 0 ) {
                break;
            } else {
                chance *= 0.66f;
            }
        }
    }

    free(valid_points);

    V_ClearRGB(0, 0, 0);
    DebugRenderTiles(map);
    DebugRenderActors(game);
    V_Refresh();

    if ( show_map_gen ) {
        DebugWaitForKeyPress();
    }
}
