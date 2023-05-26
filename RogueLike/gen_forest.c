//
//  forest.c
//  RogueLike
//
//  Created by Thomas Foster on 4/1/23.
//

#include "game.h"
#include "tile.h"
#include "mathlib.h"
#include "array.h"
#include "genlib.h"

#define FOREST_MAX_SIZE 256

struct region {
    u16 region;
    u16 area;
} regions[FOREST_MAX_SIZE * FOREST_MAX_SIZE];

//int region_areas[FOREST_MAX_SIZE * FOREST_MAX_SIZE];

static int num_coords;
static TileCoord coords[FOREST_MAX_SIZE * FOREST_MAX_SIZE]; // TODO: Use Array

static void GetTilesInRegion(const Map * map, int region)
{
    num_coords = 0;
    TileCoord coord;
    for ( coord.y = 0; coord.y < map->height; coord.y++ ) {
        for ( coord.x = 0; coord.x < map->width; coord.x++ ) {
            const Tile * tile = GetTile(map, coord);
            if ( tile->id == region ) {
                coords[num_coords++] = coord;
            }
        }
    }
}


static void AddTile(TileCoord coord)
{
    coords[num_coords++] = coord;
}


static void RemoveTile(int index)
{
    coords[index] = coords[--num_coords];
}


//static int RandomIndex(void)
//{
//    return Random(0, num_coords - 1);
//}


static Actor * SpawnActorAtRandomLocation(Game * game, ActorType type, int max_index)
{
    if ( num_coords != 0 ) {
        int index = Random(0, max_index);
        Actor * actor = SpawnActor(game, type, coords[index]);
        RemoveTile(index);
        return actor;
    } else {
        printf("%s: not tiles left!\n", __func__);
        return NULL;
    }
}


static Tile * CreateTileAtRandomLocation(Map * map, TileType type, int max_index, TileCoord * out)
{
    int index = Random(0, max_index);
    TileCoord coord = coords[index];

    if ( out ) {
        *out = coord;
    }

    Tile * tile = GetTile(map, coord);
    *tile = CreateTile(type);
    if ( area_info[AREA_FOREST].reveal_all ) {
        tile->flags.revealed = true;
    }
    RemoveTile(index);

    return tile;
}


/// Sort all connected ground tiles into regions and calculate their areas.
static void FloodFillGroundTiles_r(Map * map, TileCoord coord, int region)
{
    Tile * tile = GetTile(map, coord);
    if ( tile == NULL ) {
        printf("NULL tile\n");
        return;
    }

    if ( tile->type == TILE_FOREST_GROUND && tile->id == -1 ) {
        tile->id = region;
//        region_areas[region]++;
        regions[region].region = region;
        regions[region].area++;
        for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
            TileCoord adjacent_coord = AdjacentTileCoord(coord, d);
            FloodFillGroundTiles_r(map, adjacent_coord, region);
        }
    }
}


static int GetTilesInFirstRegionSmallerThan(const Map * map,
                                   int area,
                                   int backup_index,
                                   int num_regions)
{
    int result_area = -1;
    for ( int i = 0; i < num_regions; i++ ) {
        if ( regions[i].area < area ) {
            GetTilesInRegion(map, regions[i].region);
            result_area = regions[i].area;
            break;
        }
    }

    if ( result_area == -1 ) {
        // If a region wasn't found, use the backup.
        GetTilesInRegion(map, regions[backup_index].region);
        result_area = regions[backup_index].area;
    }

    return result_area;
}


void CalculateTileDistancesFrom(Map * map, TileCoord coord)
{
    for ( int i = 0; i < num_coords; i++ ) {
        Tile * tile = GetTile(map, coords[i]);
        tile->distance = TileDistance(coords[i], coord);
    }
}


void SortCoordsByDistance(Map * map)
{
    // Sort coords by distance, farthest first.
    for ( int i = 0; i < num_coords; i++ ) {
        for ( int j = i + 1; j < num_coords; j++ ) {
            Tile * i_tile = GetTile(map, coords[i]);
            Tile * j_tile = GetTile(map, coords[j]);

            if ( j_tile->distance > i_tile->distance ) {
                TileCoord temp = coords[i];
                coords[i] = coords[j];
                coords[j] = temp;
            }
        }
    }
}

/// Generate a forest level. The map is square.
/// - parameter width: The full width of the map.
/// - radius: The radius of the centered, circular region in the center of the
///     map inside which clearings are generated.
void GenerateForest(Game * game, int seed, int width)
{
    if ( width > FOREST_MAX_SIZE ) {
        Error("forest size must be <= %d\n", FOREST_MAX_SIZE);
    }

    printf("\n- Generate Forest- \n");
    printf("(<>) seed: %d\n", seed);
    printf("low: %0.2f\n", game->forest_low);
    printf("high: %0.2f\n", game->forest_high);
    printf("(u) freq: %.02f\n", game->forest_freq);
    printf("(i) amp: %.01f\n", game->forest_amp);
    printf("(o) pers: %.01f\n", game->forest_pers);
    printf("(p) lac: %.01f\n", game->forest_lec);

    World * world = &game->world;
    world->area = AREA_FOREST;
    world->map = &world->maps[0];

//    SDL_memset(region_areas, 0, sizeof(region_areas));
    SDL_memset(regions, 0, sizeof(regions));

    AllocateMapTiles(world->map, width, width, TILE_TREE);
    RemoveAllActors(&world->map->actor_list);

    int map_size = width * width;
    printf("Forest size: %d (%d x %d)\n", map_size, width, width);

    int radius = (width / 2) * 0.75;
    printf("Outside radius: %d tiles\n", width / 2 - radius);

    RandomizeNoise(seed);
    num_coords = 0;

    // Generate forest (tree), ground, and water terrain.
    // Add all ground tile coords to the array.
    for ( int y = 0; y < width; y++ ) {
        for ( int x = 0; x < width; x++ ) {
            TileCoord coord = { x, y };

            // Distance from this tile to center of map.
            float distance = DISTANCE(x, y, width / 2, width / 2);

            float noise;
            float water_noise;

            if ( distance <= radius ) {
                float gradient = MAP(distance, 0.0f, (float)radius, 0.0f, 1.0f);
                float freq = game->forest_freq;
                float amp = game->forest_amp;
                float pers = game->forest_pers;
                float lac = game->forest_lec;
                noise = Noise2(x, y, 1.0f, freq, 6, amp, pers, lac) - gradient;

                water_noise
                = Noise2(x, y, 8241931.0f, freq, 6, amp, pers, lac) - gradient;
            } else {
                noise = -1.0f; // Outside level radius.
                water_noise = -1.0f;
            }

            Tile * tile = GetTile(world->map, coord);

            if ( noise < game->forest_low || noise > game->forest_high ) {
                *tile = CreateTile(TILE_TREE);
            } else {
                *tile = CreateTile(TILE_FOREST_GROUND);
            }

            if ( water_noise > game->forest_high ) {
                *tile = CreateTile(TILE_WATER);
            }

            if ( tile->type == TILE_FOREST_GROUND ) {
                AddTile(coord);
            }

            tile->id = -1; // Reset all tiles' region
            if ( area_info[AREA_FOREST].reveal_all ) {
                tile->flags.revealed = true;
            }
        }
    }

    // For all ground tiles, sort into connected regions.
    int region = -1;
    for ( int i = 0; i < num_coords; i++ ) {
        Tile * tile = GetTile(world->map, coords[i]);

        if ( tile->id == -1 ) { // Not yet visited
            region++;
            FloodFillGroundTiles_r(world->map, coords[i], region);
        }
    }

    int num_regions = region + 1;
    num_coords = 0;

    // Sort regions by highest area.
    for ( int i = 0; i < num_regions ; i++ ) {
        for ( int j = i + 1; j < num_regions; j++ ) {
            if ( i != j && regions[j].area > regions[i].area ) {
                struct region temp = regions[i];
                regions[i] = regions[j];
                regions[j] = temp;
            }
        }
    }

    for ( int i = 0; i < region; i++ ) {
        printf("- region %d: area %d\n", regions[i].region, regions[i].area);
    }

    // TODO: refactor ASAP
    //
    // Starting region
    // Just the player and a teleporter.
    //

    int area = GetTilesInFirstRegionSmallerThan(world->map, 80, 3, num_regions);

    Actor * player = SpawnActorAtRandomLocation(game, ACTOR_PLAYER, num_coords - 1);

    CalculateTileDistancesFrom(world->map, player->tile);
    SortCoordsByDistance(world->map);
    int num_viable = area / 10;
    Tile * tp = CreateTileAtRandomLocation(world->map, TILE_TELEPORTER, num_viable, NULL);
    tp->tag = 0;

    SpawnActorAtRandomLocation(game, ACTOR_SUPER_SPIDER, num_coords / 20);

    //
    // Second region - small area
    // Spawn spiders, and teleporter
    //

    area = GetTilesInFirstRegionSmallerThan(world->map, 128, 2, num_regions);

    // Create first teleporter.
    TileCoord tp_coord;
    tp = CreateTileAtRandomLocation(world->map, TILE_TELEPORTER, num_coords - 1, & tp_coord);
    tp->tag = 0;

    // Create second teleporter.
    CalculateTileDistancesFrom(world->map, tp_coord);
    SortCoordsByDistance(world->map);
    num_viable = area / 10;
    tp = CreateTileAtRandomLocation(world->map, TILE_TELEPORTER, num_viable, NULL);
    tp->tag = 1;

    for ( int i = 0; i < area / 20; i++ ) {
        SpawnActorAtRandomLocation(game, ACTOR_SPIDER, num_coords - 1);
    }

    //
    // Third region - medium size
    // Spawn second teleporter, spiders, super spiders.
    //

    area = GetTilesInFirstRegionSmallerThan(world->map, 256, 1, num_regions);

    // Create first teleporter.
    tp = CreateTileAtRandomLocation(world->map, TILE_TELEPORTER, num_coords - 1, &tp_coord);
    tp->tag = 1; // connected to previous region

    // Create second teleporter.
    CalculateTileDistancesFrom(world->map, tp_coord);
    SortCoordsByDistance(world->map);
    num_viable = area / 10;
    tp = CreateTileAtRandomLocation(world->map, TILE_TELEPORTER, num_viable, NULL);
    tp->tag = 2; // connected to next region

    // Spawn the shack in an open area so the player can get around it.
    Array * viable_shack_spots = NewArray(num_coords, sizeof(TileCoord), 0);
    for ( int i = 0; i < num_coords; i++ ) {
        TileCoord coord = coords[i];
        for ( int d = 0; d < NUM_DIRECTIONS; d++ ) {
            Tile * adj = GetAdjacentTile(world->map, coord, d);
            if ( adj->type != TILE_FOREST_GROUND ) {
                goto next_coord;
            }
        }
        // This tile is completely surrounded by ground.
        Push(viable_shack_spots, &coord);
    next_coord:
        ;
    }

    int shack_index = Random(0, viable_shack_spots->count - 1);
    TileCoord * spawn_spot = Get(viable_shack_spots, shack_index);
    SpawnActor(game, ACTOR_SHACK_CLOSED, *spawn_spot);
//    player->tile = *spawn_spot; // DEBUG

    FreeArray(viable_shack_spots);

    // Spawn spiders
    for ( int i = 0; i < area / 10; i++ ) {
        if ( Chance(0.2) ) {
            SpawnActorAtRandomLocation(game, ACTOR_SUPER_SPIDER, num_coords - 1);
        } else {
            SpawnActorAtRandomLocation(game, ACTOR_SPIDER, num_coords - 1);
        }
    }

    //
    // Fourth & Final region - largest
    // Spawn ...
    // Spawn the exit.
    //

    GetTilesInRegion(world->map, regions[0].region);
    area = regions[0].area;

    tp = CreateTileAtRandomLocation(world->map, TILE_TELEPORTER, num_coords - 1, &tp_coord);
    tp->tag = 2; // connected to previos region

    CalculateTileDistancesFrom(world->map, tp_coord);
    SortCoordsByDistance(world->map);
    num_viable = area / 10;

    int index = Random(0, num_viable);
    TileCoord exit_coord = coords[index];
    Tile * tile = GetTile(world->map, exit_coord);
    *tile = CreateTile(TILE_FOREST_EXIT);
    if ( area_info[AREA_FOREST].reveal_all ) {
        tile->flags.revealed = true;
    }
    SpawnActor(game, ACTOR_WELL, exit_coord);

    for ( int i = 0; i < area / 10; i++ ) {
        // TODO: tweak
        if ( Chance(0.05) ) {
            SpawnActorAtRandomLocation(game, ACTOR_GHOST, num_coords - 1);
        } else if ( Chance( 0.1 ) ) {
            SpawnActorAtRandomLocation(game, ACTOR_SUPER_SPIDER, num_coords - 1);
        } else {
            SpawnActorAtRandomLocation(game, ACTOR_SPIDER, num_coords - 1);
        }
    }

    SpawnActorAtRandomLocation(game, ACTOR_OLD_KEY, num_coords - 1);

    //
    // Generate shack interior
    //

#define SHACK_WIDTH 11
#define SHACK_HEIGHT 11

    // Extremely sophisticated design technique!
    // TODO: expand this, more interesting shape:
    // TODO: decoration actors: cobwebs,
    // (eg two rooms connected by corridor)
    const char shack_map[SHACK_WIDTH * SHACK_HEIGHT] = {
        "0XXXX@XXX00"
        "0........00"
        "0........X0"
        "0.........0"
        "0.........0"
        "0.........0"
        "00........0"
        "00........0"
        "00........0"
        "000......00"
        "00000000000"
    };

    world->map++;

    int shack_size = SHACK_WIDTH * SHACK_HEIGHT;
    AllocateMapTiles(world->map, SHACK_WIDTH, SHACK_HEIGHT, TILE_WOODEN_FLOOR);
    RemoveAllActors(&world->map->actor_list);

    // A list of possible spawn locations for mobs and the bucket.
    Array * shack_coords_array = NewArray(shack_size, sizeof(TileCoord), 0);

    const char * c = shack_map;
    for ( int i = 0; i < shack_size; i++, c++ ) {
        TileCoord coord = GetCoordinate(world->map, i);
        tile = &world->map->tiles[i];

        switch ( *c ) {
            case '0':
                *tile = CreateTile(TILE_NULL);
                break;
            case 'X':
                *tile = CreateTile(TILE_WOODEN_WALL);
                break;
            case '.':
                if ( TileDistance(player->tile, coord) >= 3 ) {
                    Push(shack_coords_array, &coord);
                }
                break;
            case '@':
                *tile = CreateTile(TILE_WHITE_OPENING);
                player = SpawnActor(game, ACTOR_PLAYER, coord);
                break;
            default: {
                ASSERT("Weird character in shack!");
            }
        }

    }

    // Spawn the bucket.
    TileCoord * shack_coords = shack_coords_array->data;
    index = Random(0, shack_coords_array->count - 1);
    SpawnActor(game, ACTOR_ROPE, shack_coords[index]);
    Remove(shack_coords_array, index);

    // Spawn a couple ghosts
    int num_ghosts = Random(2, 3);
    for ( int i = 0; i < num_ghosts; i++ ) {
        index = Random(0, shack_coords_array->count - 1);
        SpawnActor(game, ACTOR_GHOST, shack_coords[index]);
        Remove(shack_coords_array, index);
    }

    FreeArray(shack_coords_array);

    world->map--;
}
