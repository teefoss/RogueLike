//
//  gen.c
//  RogueLike
//
//  Created by Thomas Foster on 11/4/22.
//

#include "game.h"
#include "mathlib.h"
#include "texture.h"
#include "video.h"

const int debug_tile_size = 16;

static TileCoord * _buffer;
static int _count;


static void BufferClear(void)
{
    _count = 0;
}


static void BufferAppend(TileCoord coord)
{
    _buffer[_count++] = coord;
}


static void BufferRemove(int index)
{
    _buffer[index] = _buffer[--_count];
}


//static TileCoord BufferRandom(void)
//{
//    return _buffer[Random(0, _count - 1)];
//}


static int RandomIndex(void)
{
    return Random(0, _count - 1);
}


#pragma mark -


TileID * GetTileID(Map * map, TileCoord coord)
{
    return &map->tile_ids[coord.y * map->width + coord.x];
}


void DebugRenderTiles(const Map * map, Area area, int tile_size, const RenderInfo * info)
{
    for ( int y = 0; y < map->height; y++ ) {
        for ( int x = 0; x < map->width; x++ ) {
            Tile * tile = GetTile((Map *)map, ((TileCoord){ x, y }));
            RenderTile(tile,
                       area,
                       0,
                       x * tile_size,
                       y * tile_size,
                       tile_size,
                       true,
                       info);
        }
    }
}


void RenderTilesWithDelay(Game * game)
{
    if ( show_map_gen ) {
        CheckForShowMapGenCancel();
        V_ClearRGB(0, 0, 0);
        DebugRenderTiles(game->world.map,
                         game->world.area,
                         game->world.info->debug_map_tile_size,
                         &game->render_info);
        V_Refresh();
        SDL_Delay(25);
    }
}


///
/// Carve out a hallway recursively, setting each new floor tile with the
/// current region ID.
///
static void GenerateHallway_r(Map * map, int current_id, TileCoord coord)
{
    Tile * here = GetTile(map, coord);
    TileID * id = GetTileID(map, coord);
    *here = CreateTile(TILE_DUNGEON_FLOOR);
    here->id = -1;
    *id = current_id;

//    RenderTilesWithDelay(map);

    struct {
        TileCoord coord;
        Direction direction;
    } directions[NUM_CARDINAL_DIRECTIONS];

    int num_open_directions = 0;

    // Make a list of the coordinates of possible directions.
    for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
//        int next_x = x + x_deltas[i] * 2;
//        int next_y = y + y_deltas[i] * 2;
//        tile_t * next = GetTile(map, next_x, next_y);
        TileCoord next_coord = {
            coord.x + XDelta(d) * 2,
            coord.y + YDelta(d) * 2
        };
        Tile * next = GetTile(map, next_coord);

        if ( next && next->type == TILE_DUNGEON_WALL ) {
//        if ( IsInBounds(map, next_x, next_y) && next->type == TILE_WALL ) {
            directions[num_open_directions].coord = next_coord;
            directions[num_open_directions].direction = d;
            num_open_directions++;
        }
    }

    if ( num_open_directions == 0 ) {
        return;
    }

    // Pick a random direction and open it up.
    int i = Random(0, num_open_directions - 1);
    Direction dir = directions[i].direction;
    TileCoord next_coord = directions[i].coord;

    // Clear the spot in between this and the next.
    TileCoord inbetween_coord = {
        next_coord.x - XDelta(dir),
        next_coord.y - YDelta(dir)
    };
//    int inbetween_x = next.x - x_deltas[dir];
//    int inbetween_y = next.y - y_deltas[dir];
//    map[inbetween_y][inbetween_x] = CreateTile(TILE_FLOOR);
//    ids[inbetween_y][inbetween_x] = current_id;
    Tile * inbetween = GetTile(map, inbetween_coord);
    *inbetween = CreateTile(TILE_DUNGEON_FLOOR);
    TileID * inbetween_id = GetTileID(map, inbetween_coord);
    *inbetween_id = current_id;

//    RenderTilesWithDelay(map);

    GenerateHallway_r(map, current_id, next_coord);
}




/// Change all `from` IDs to `to`.
void ChangeAllIDs(Map * map, TileID from, TileID to)
{
    for ( int i = 0; i < map->width * map->height; i++ ) {
        if ( map->tile_ids[i] == from ) {
            map->tile_ids[i] = to;
        }
    }
}




// A map tile that is adjacent to the main region and another different region.
typedef struct {
    TileCoord coord;
    int region; // the other region
    bool valid;
} connector_t;

#define MAP_EDGE_ID (-2)
#define MAP_WALL_ID (-1)

int GetConnectors(Map * map, int main_region, connector_t * out)
{
    int num_connectors = 0;

    TileCoord coord;
    for ( coord.y = 1; coord.y < map->height - 2; coord.y++ ) {
        for ( coord.x = 1; coord.x < map->width - 2; coord.x++ ) {

            // Potential connectors must be wall tiles.
            if ( *GetTileID(map, coord) != MAP_WALL_ID ) {
                continue;
            }

            // Check all four adjacent tiles to see if (x, y) touches the main
            // region and another region.
            bool touches_main = false;
            int other_region = -1;

            for ( int d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
                TileCoord adjacent = AdjacentTileCoord(coord, d);

                TileID id = *GetTileID(map, adjacent);
                if ( id == main_region ) {
                    touches_main = true;
                }

                if ( id >= 0 && id != main_region ) {
                    other_region = id;
                }
            }

            if ( touches_main && other_region != -1 ) {
                // It's valid, add to list.
                out[num_connectors].coord = coord;
                out[num_connectors].region = other_region;
                num_connectors++;
            }
        }
    }

    return num_connectors;
}




/// Get an array of coordinates of tiles that are dead ends.
int GetDeadEnds(Map * map, TileCoord * out)
{
    int count = 0;

    for ( int y = 1; y < map->height - 1; y++ ) {
        for ( int x = 1; x < map->width - 1; x++ ) {
            TileCoord coord = { x, y };
            Tile * tile = GetTile(map, coord);

            if ( tile->type == TILE_DUNGEON_FLOOR ) {

                // Count the number of non-wall connections to this tile.
                int connection_count = 0;
                for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
//                    tile_t * adj = GetTile(map, x + x_deltas[d], y + y_deltas[d]);
                    Tile * adj = GetAdjacentTile(map, coord, d);
                    if ( adj->type != TILE_DUNGEON_WALL ) {
                        connection_count++;
                    }
                }

                if ( connection_count == 1 ) { // It's a dead end.
                    out[count] = coord;
                    count++;
                }
            }
        }
    }

    return count;
}


static void GetRoomCorners(SDL_Rect room, TileCoord corners[4])
{
    int top = room.y;
    int left = room.x;
    int right = room.x + room.w - 1;
    int bottom = room.y + room.h - 1;

    corners[0] = (TileCoord){ left,  top };
    corners[1] = (TileCoord){ right, top };
    corners[2] = (TileCoord){ left,  bottom };
    corners[3] = (TileCoord){ right, bottom };
}

#if 0
static SDL_Point GetRandomPointInRoom(map_t * map, int room_num)
{
    SDL_Rect room = map->rooms[room_num];
    SDL_Point pt;
    pt.x = Random(room.x, room.x + room.w - 1);
    pt.y = Random(room.y, room.y + room.h - 1);

    return pt;
}


static tile_coord_t GetRandomRoomCorner(map_t * map, int room_num)
{
    SDL_Rect room = map->rooms[room_num];
    tile_coord_t corners[4];
    GetRoomCorners(room, corners);
    return corners[Random(0, 3)];
}
#endif

TileCoord GetRoomCenter(SDL_Rect room)
{
    TileCoord center = { room.x + room.w / 2, room.y + room.h / 2 };
    return center;
}


///
/// Get a list of tiles in a room that are unoccupied by an actor and not
/// adjacent to a door.
///
static void GetValidRoomTiles(const World * world, int room_num)
{
    const Map * map = world->map;
    bool * occupied = calloc(map->width * map->height, sizeof(*occupied));

    FOR_EACH_ACTOR_CONST(actor, world->map->actor_list) {
        int x = actor->tile.x;
        int y = actor->tile.y;
        occupied[y * map->width + x] = true;
    }

    SDL_Rect room = map->rooms[room_num];
    BufferClear();

    TileCoord coord;
    for ( coord.y = room.y; coord.y < room.y + room.h; coord.y++ ) {
        for ( coord.x = room.x; coord.x < room.x + room.w; coord.x++ ) {

            bool valid = true;

            // Is it a floor tile?
            const Tile * tile = GetTile(map, coord);
            if ( tile->type != TILE_DUNGEON_FLOOR ) {
                valid = false;
            }

            // Actor there already?
            if ( occupied[coord.y * map->width + coord.x] ) {
                valid = false;
            }

            // Adjacent to door?
            for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
                const Tile * t = GetAdjacentTile((Map *)map, coord, d);

                if ( t->type == TILE_DUNGEON_DOOR || t->type == TILE_GOLD_DOOR ) {
                    valid = false;
                    break;
                }
            }

            if ( valid ) {
                BufferAppend(coord);
            }
        }
    }

    free(occupied);
}


static void GetReachableTiles(Map * map, TileCoord start, int ignore_flags)
{
//    CalculateDistances(map, start, ignore_flags);

    BufferClear();
    for ( int i = 0; i < map->width * map->height; i++ ) {
        if ( map->tiles[i].distance >= 0 ) {
            BufferAppend(GetCoordinate(map, i));
        }
    }
}


static void InitTiles(Map * map)
{
    TileCoord coord;
    for ( coord.y = 0; coord.y < map->height; coord.y++ ) {
        for ( coord.x = 0; coord.x < map->width; coord.x++ ) {

            Tile * t = GetTile(map, coord);
            TileID * id = GetTileID(map, coord);

            *t = CreateTile(TILE_DUNGEON_WALL);
            t->id = -1;

            if (   coord.x == 0
                || coord.x == map->width - 1
                || coord.y == 0
                || coord.y == map->height - 1 )
            {
                *id = MAP_EDGE_ID;
            } else {
                *id = MAP_WALL_ID;
            }
        }
    }
}


void SpawnRooms(Map * map, int * current_id)
{
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

        rect.x = Random(1, (map->width - rect.w) / 2) * 2 - 1;
        rect.y = Random(1, (map->height - rect.h) / 2) * 2 - 1;

        // Check that this potential room does not overlap with any
        // existing rooms.
        for ( int room = 0; room < map->num_rooms; room++ ) {
            if ( SDL_HasIntersection(&rect, &map->rooms[room]) ) {
                goto next_try;
            }
        }

        // Spot is clear. Open up the room and set its floor tiles to the
        // current region ID.
        map->rooms[map->num_rooms] = rect;
        TileCoord coord;
        for ( coord.y = rect.y; coord.y < rect.y + rect.h; coord.y++ ) {
            for ( coord.x = rect.x; coord.x < rect.x + rect.w; coord.x++ ) {
                Tile * tile = GetTile(map, coord);
                *tile = CreateTile(TILE_DUNGEON_FLOOR);
//                tile->flags |= FLAG(TILE_ROOM);
                tile->id = map->num_rooms;
                *GetTileID(map, coord) = *current_id;
            }
        }
        printf("placed a room with id %d (%d, %d, %d, %d)\n",
               *current_id,
               rect.x, rect.y, rect.w, rect.h);
        map->num_rooms++;
        (*current_id)++;

//        RenderTilesWithDelay(map);

    next_try:
        ;
    }
}


void GenerateHallways(Map * map, int * current_id)
{
    do {
        BufferClear();

        // Check all odd positions and make a list of viable spots at
        // which to begin a hallway.
        TileCoord coord;
        for ( coord.y = 1; coord.y <= map->height - 2; coord.y += 2 ) {
            for ( coord.x = 1; coord.x <= map->width - 2; coord.x += 2 ) {
                if ( GetTile(map, coord)->type == TILE_DUNGEON_WALL ) {
                    BufferAppend(coord);
                }
            }
        }

        if ( _count > 0 ) {
            // There are still spots at which to start a hallway. Select
            // a random one and begin carving it out.
            int i = Random(0, _count - 1);
            GenerateHallway_r(map, *current_id, _buffer[i]);
            (*current_id)++;
        }
    } while ( _count > 0 );
}


int ConnectRegions(Map * map, TileCoord * potential_door_locations, int num_regions)
{
    // Pick a random region to start.
    int main_region = Random(0, num_regions - 1);

    // While there are still connectors adjacent to the main region,
    connector_t * connectors = calloc(map->width * map->height, sizeof(*connectors));
    int num_connectors = 0;
    int num_potential_door_locations = 0;
    while ( (num_connectors = GetConnectors(map, main_region, connectors)) ) {

        // Pick a random connector, open it up, and merge the other region with
        // the main.
        connector_t connector = connectors[Random(0, num_connectors - 1)];

        Tile * tile = GetTile(map, connector.coord);
        *tile = CreateTile(TILE_DUNGEON_FLOOR);

        potential_door_locations[num_potential_door_locations++] = connector.coord;
        ChangeAllIDs(map, connector.region, main_region);

//        RenderTilesWithDelay(map);
    }

    free(connectors);

    return num_potential_door_locations;
}


void EliminateDeadEnds(Map * map)
{
    int num_deadends = 0;
    TileCoord * deadends = calloc(map->width * map->height, sizeof(*deadends));

    while ( (num_deadends = GetDeadEnds(map, deadends)) ) {
        for ( int i = 0; i < num_deadends; i++ ) {
            *GetTile(map, deadends[i]) = CreateTile(TILE_DUNGEON_WALL);
//            RenderTilesWithDelay(map);
        }
    }

    free(deadends);
}


void SpawnDoors(Map * map, TileCoord * potentials, int array_len)
{
    for ( int i = 0; i < array_len; i++ ) {
        Tile * tile = GetTile(map, potentials[i]);

        Tile * adjacents[NUM_CARDINAL_DIRECTIONS];
        for ( int d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
            adjacents[d] = GetAdjacentTile(map, potentials[i], d);
        }

        bool is_valid =
            (adjacents[NORTH]->type == TILE_DUNGEON_FLOOR && adjacents[SOUTH]->type == TILE_DUNGEON_FLOOR)
            || (adjacents[WEST]->type == TILE_DUNGEON_FLOOR && adjacents[EAST]->type == TILE_DUNGEON_FLOOR);

        if ( tile->type == TILE_DUNGEON_FLOOR && is_valid ) {
            *tile = CreateTile(TILE_DUNGEON_DOOR);
//            RenderTilesWithDelay(map);
        }
    }

#if 0
    tile_coord_t coord;
    for ( coord.y = 0; coord.y < map->height; coord.y++ ) {
        for ( coord.x = 0; coord.x < map->width; coord.x++ ) {

            if ( GetTile(map, coord)->type == TILE_DOOR ) {

                tile_t * adjacents[NUM_CARDINAL_DIRECTIONS];
                for ( int d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
                    adjacents[d] = GetAdjacentTile(map, coord, d);
                }

                // Deadend-clearing may have rendered some door spots invalid.
                // Check if still valid.
                if ((adjacents[NORTH]->type == TILE_FLOOR && adjacents[SOUTH]->type == TILE_FLOOR)
                    || (adjacents[WEST]->type == TILE_FLOOR && adjacents[EAST]->type == TILE_FLOOR))
                {
                    // Valid, do nothing.
                } else {
                    // This spot is no longer a valid door.
                    *GetTile(map, coord) = CreateTile(TILE_WALL);
                    RenderTilesWithDelay(map);
                }
            }
        }
    }
#endif
}


void SpawnPlayerAndStartTile(Game * game)
{
    TileCoord pt = GetRoomCenter(game->world.map->rooms[0]);
    printf("player start: %d, %d\n", pt.x, pt.y);

    SpawnActor(game, ACTOR_PLAYER, pt);
    *GetTile(game->world.map, pt) = CreateTile(TILE_START);
}


void SpawnGoldKey(Game * game)
{
    Actor * player = FindActor(&game->world.map->actor_list, ACTOR_PLAYER);
    Map * map = game->world.map;

    GetReachableTiles(map, player->tile, FLAG(TILE_DUNGEON_DOOR));

    // Remove any points that are not in a room (-1) or are in the start room (0)
    for ( int i = _count - 1; i >= 0; i-- ) {
        Tile * tile = GetTile(map, _buffer[i]);
        if ( tile->id <= 0 ) {
            BufferRemove(i);
        }
    }

    TileCoord gold_key_tile_coord;

    if ( _count == 0 ) {
        puts("Could not find spot for gold key!");
        // The start room's only door is to the exit room.
        // For now, just spawn the key in the start room (lame).
        GetValidRoomTiles(&game->world, 0);
    }

    gold_key_tile_coord = _buffer[Random(0, _count - 1)];

    SpawnActor(game, ACTOR_GOLD_KEY, gold_key_tile_coord);

    // Save the gold key's room number.
    Tile * gold_key_tile = GetTile(map, gold_key_tile_coord);
    map->gold_key_room_num = gold_key_tile->id;
}


///
/// Try to place the exit in the corner of a room, choosing a corner that
/// is not adjacent to a door. If none are found, place the exit in the center
/// of the room.
///
void SpawnExit(Game * game)
{
    Map * map = game->world.map;

    int exit_room = map->num_rooms - 1;

    TileCoord corners[4];
    GetRoomCorners(map->rooms[exit_room], corners);

    // Make a list of viable corners.
    int num_usable = 0;
    int usable_indices[4] = { 0 };

    for ( int i = 0; i < 4; i++ ) {
        if ( !TileIsAdjacentTo(map,
                               corners[i],
                               TILE_DUNGEON_DOOR,
                               NUM_CARDINAL_DIRECTIONS) )
        {
            usable_indices[num_usable++] = i;
        }
    }

    TileCoord exit_coord;

    if ( num_usable == 0 ) {
        exit_coord = GetRoomCenter(map->rooms[exit_room]);
    } else {
        int index = Random(0, num_usable - 1);
        exit_coord = corners[usable_indices[index]];
    }

    *GetTile(map, exit_coord) = CreateTile(TILE_DUNGEON_EXIT);

    // Spawn blocks adjacent to exit stairs.
    for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
        Tile * adjacent = GetAdjacentTile(map, exit_coord, d);
        TileCoord coord = AdjacentTileCoord(exit_coord, d);
        if ( adjacent->type == TILE_DUNGEON_FLOOR ) {
            SpawnActor(game, ACTOR_PILLAR, coord);
        }
    }

    // Turn any doors adjacent to the exit room into gold doors.
    SDL_Rect rect = map->rooms[exit_room];

    // Expand by one to include any doors touching the exit room.
    rect.x--;
    rect.y--;
    rect.w++;
    rect.h++;

    // TODO: do this not dumb.
    for ( int y = rect.y; y <= rect.y + rect.h; y++ ) {
        for ( int x = rect.x; x <= rect.x + rect.w; x++ ) {
            Tile * tile = GetTile(map, ((TileCoord){ x, y }));
            if ( tile->type == TILE_DUNGEON_DOOR ) {
                *tile = CreateTile(TILE_GOLD_DOOR);
            }
        }
    }
}


void SpawnActorAtRandomPointInBuffer(Game * game, ActorType type)
{
    if ( _count > 0 ) {
        int i = Random(0, _count - 1);
        SpawnActor(game, type, _buffer[i]);
        BufferRemove(i);
    } else {
        printf("no room to spawn %s!", actor_info_list[type].name);
    }
}


// https://www.tomstephensondeveloper.co.uk/post/creating-simple-procedural-dungeon-generation
void GenerateDungeon(Game * game, int width, int height)
{
    if ( width % 2 == 0 || height % 2 == 0 ) {
        Error("Dugeon width and height must be odd");
    }

    game->world.area = AREA_DUNGEON;

    Map * map = game->world.map;
    map->width = width;
    map->height = height;

    //
    // Init tiles.
    //

    // IDs are unique identifers for each 'region'. Regions are
    // any unconnected spaces. Unset tiles start with a negative ID indicating
    // it's still a floor, or a floor on the edge of the map.

    int map_size = width * height;

    _count = 0;
    _buffer = calloc(map_size, sizeof(*_buffer));
    if ( _buffer == NULL ) {
        Error("Could not allocate tile coord buffer");
    }

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

    InitTiles(map);

    map->num_rooms = 0;
    int current_id = 0; // Regions

    SpawnRooms(map, &current_id);

    GenerateHallways(map, &current_id);

    const int num_regions = current_id;

    TileCoord * potential_door_locations = calloc(map_size, sizeof(*potential_door_locations));
    int num_potential_door_locations = ConnectRegions(map, potential_door_locations, num_regions);
    EliminateDeadEnds(map);
    SpawnDoors(map, potential_door_locations, num_potential_door_locations);
    free(potential_door_locations);
    SpawnExit(game);

    // Spawn actors.

    SpawnPlayerAndStartTile(game);


    // Pick a room that is not the start or end room for the button.
    int button_room_num = Random(1, map->num_rooms - 2);

    // Get Reachable Tiles:
    // - ignoring doors, gold doors (but not exit pillars)

    SpawnGoldKey(game);

    // In each room...
    for ( int i = 0; i < map->num_rooms; i++ ) {
        GetValidRoomTiles(&game->world, i);
        float chance = 1.0f;
        int area = map->rooms[i].w * map->rooms[i].h;
        int max_monsters = area / 4;

        if ( button_room_num == i ) {
//            SpawnActor(game, ACTOR_BUTTON_UP, BufferRandom());
            int index = RandomIndex();
            Tile * tile = GetTile(map, _buffer[index]);
            *tile = CreateTile(TILE_BUTTON_NOT_PRESSED);
            BufferRemove(index);
        }

        for ( int j = 0; j < 4; j++ ) {
            if ( Chance(chance) ) {
                SpawnActorAtRandomPointInBuffer(game, ACTOR_BLOB);
                --max_monsters;
            }

            if ( max_monsters == 0 ) {
                break;
            } else {
                chance *= 0.66f;
            }
        }

        int max_vases = area / 9;
        for ( int j = 0; j < max_vases; j++ ) {
            SpawnActorAtRandomPointInBuffer(game, ACTOR_VASE);
        }

        SpawnActorAtRandomPointInBuffer(game, ACTOR_CLOSED_CHEST);
    }

    if ( show_map_gen ) {
        DebugWaitForKeyPress();
    }
}
