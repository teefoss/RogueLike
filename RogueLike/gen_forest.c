//
//  forest.c
//  RogueLike
//
//  Created by Thomas Foster on 4/1/23.
//

#include "game.h"
#include "tile.h"
#include "mathlib.h"

#define FOREST_MAX_SIZE 256

int region_areas[FOREST_MAX_SIZE * FOREST_MAX_SIZE];

static STORAGE(TileCoord, FOREST_MAX_SIZE * FOREST_MAX_SIZE) ground_coords;
static int distances[FOREST_MAX_SIZE][FOREST_MAX_SIZE];

static int num_tiles;
static TileCoord tiles[FOREST_MAX_SIZE * FOREST_MAX_SIZE];

static void GetTilesInRegion(const Map * map, int region)
{
    num_tiles = 0;
    TileCoord coord;
    for ( coord.y = 0; coord.y < map->height; coord.y++ ) {
        for ( coord.x = 0; coord.x < map->width; coord.x++ ) {
            const Tile * tile = GetTile(map, coord);
            if ( tile->id == region ) {
                tiles[num_tiles++] = coord;
            }
        }
    }
}


static void RemoveTile(int index)
{
    tiles[index] = tiles[--num_tiles];
}


static int RandomIndex(void)
{
    return Random(0, num_tiles - 1);
}


static void SpawnActorAtRandomLocation(Game * game, ActorType type)
{
    if ( num_tiles != 0 ) {
        int index = RandomIndex();
        SpawnActor(game, type, tiles[index]);
        RemoveTile(index);
    } else {
        printf("%s: not tiles left!\n", __func__);
    }
}


static TileCoord CreateTileAtRandomLocation(Map * map, TileType type)
{
    int index = RandomIndex();
    TileCoord coord = tiles[index];

    Tile * tile = GetTile(map, coord);
    *tile = CreateTile(type);
    tile->flags.revealed = true;
    RemoveTile(index);

    return coord;
}


/// Sort all connected ground tiles into regions and calculate their areas.
static void FloodFillGroundTiles_r(Map * map, TileCoord coord, int region)
{
    Tile * tile = GetTile(map, coord);
    if ( tile == NULL ) {
        printf("NULL tile\n");
        return;
    }

    if ( tile->type == TILE_FLOOR && tile->id == -1 ) {
        tile->id = region;
        region_areas[region]++;
        for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
            TileCoord adjacent_coord = AdjacentTileCoord(coord, d);
            FloodFillGroundTiles_r(map, adjacent_coord, region);
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
    printf("seed: %d\n", seed);
    printf("low: %0.2f\n", game->forest_low);
    printf("high: %0.2f\n", game->forest_high);
    printf("freq: %.02f\n", game->forest_freq);
    printf("amp: %.01f\n", game->forest_amp);
    printf("pers: %.01f\n", game->forest_pers);
    printf("lac: %.01f\n", game->forest_lec);

    World * world = &game->world;
    world->area = AREA_FOREST;


    SDL_memset(region_areas, 0, sizeof(region_areas));

    Map * map = &world->map;

    map->width = width;
    map->height = width;
    int map_size = width * width;
    printf("Forest size: %d (%d x %d)\n", map_size, map->width, map->height);
    int radius = (width / 2) * 0.75;
    printf("Outside radius: %d tiles\n", width / 2 - radius);

    if ( map->tiles ) {
        free(map->tiles);
    }
    map->tiles = calloc(map_size, sizeof(*map->tiles));

    for ( int i = 0; i < map_size; i++ ) {
        map->tiles[i] = CreateTile(TILE_WALL);
    }

    RandomizeNoise(seed);
//    RandomizeNoise(0);
    CLEAR(ground_coords);

    // Generate forest (wall), ground (floor), and water terrain.
    // Spawn trees on wall tiles.
    for ( int y = 0; y < map->height; y++ ) {
        for ( int x = 0; x < map->width; x++ ) {
            TileCoord coord = { x, y };

            // Distance from this tile to center of map.
            float distance = DISTANCE(x, y, width / 2, width / 2);
            distances[y][x] = distance;

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

            Tile * tile = GetTile(map, coord);

            if ( noise < game->forest_low || noise > game->forest_high ) {
                *tile = CreateTile(TILE_TREE);
            } else {
                *tile = CreateTile(TILE_FLOOR);
            }

            if ( water_noise > game->forest_high ) {
                *tile = CreateTile(TILE_WATER);
            }

            if ( tile->type == TILE_FLOOR ) {
                APPEND(ground_coords, coord);
            }

            tile->id = -1; // Reset all tiles' region
            tile->flags.revealed = true;
        }
    }

    // For all ground tiles, sort into connected regions.
    int region = -1;
    for ( int i = 0; i < ground_coords.count; i++ ) {
        TileCoord coord = ground_coords.data[i];
        Tile * tile = GetTile(map, ground_coords.data[i]);
        if ( tile->id == -1 ) { // Not yet visited
            region++;
            FloodFillGroundTiles_r(map, coord, region);
//            printf("region %d area: %d\n", region, region_areas[region]);
        }
    }

    printf("total regions: %d\n", region);
    int largest_region = -1;
    int second_largest_region = -1;
    int max = -1;
    int max2 = -1;

    // Find the largest and second largest region.
    for ( int i = 0; i <= region; i++ ) {
        if ( region_areas[i] > max ) {
            max2 = max;
            second_largest_region = largest_region;
            max = region_areas[i];
            largest_region = i;
        } else if ( region_areas[i] > max2 ) {
            max2 = region_areas[i];
            second_largest_region = i;
        }
    }

    printf("largest: region %d, area %d\n",
           largest_region, region_areas[largest_region]);
    printf("second largest: region %d, area %d\n",
           second_largest_region, region_areas[second_largest_region]);

    // Spawn actor, spiders, and teleporter in second largest region.
    GetTilesInRegion(map, second_largest_region); // OK

    SpawnActorAtRandomLocation(game, ACTOR_PLAYER);
    CreateTileAtRandomLocation(&world->map, TILE_TELEPORTER);
    for ( int i = 0; i < region_areas[second_largest_region] / 20; i++ ) {
        SpawnActorAtRandomLocation(game, ACTOR_SPIDER);
    }

    // Spawn second teleporter, spiders, and exit in largest region.
    GetTilesInRegion(map, largest_region);
    CreateTileAtRandomLocation(map, TILE_TELEPORTER);
    TileCoord exit_coord = CreateTileAtRandomLocation(map, TILE_EXIT);
    SpawnActor(game, ACTOR_WELL, exit_coord);
    for ( int i = 0; i < region_areas[largest_region] / 10; i++ ) {
        if ( Chance(0.2) ) {
            SpawnActorAtRandomLocation(game, ACTOR_SUPER_SPIDER);
        } else {
            SpawnActorAtRandomLocation(game, ACTOR_SPIDER);
        }
    }
}
