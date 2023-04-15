//
//  map.h
//  RogueLike
//
//  Created by Thomas Foster on 4/14/23.
//

#ifndef map_h
#define map_h

#include "mathlib.h"

#define MAX_ROOMS 200

typedef struct {
    int count;
    Actor list[MAX_ACTORS];
} Actors;

typedef struct
{
    int width;
    int height;

    Tile * tiles;
    TileID * tile_ids;

    int num_rooms;
    SDL_Rect rooms[MAX_ROOMS];

    Actors actors;
} Map;

Tile * GetAdjacentTile(Map * map, TileCoord coord, Direction direction);
Tile * GetTile(Map * map, TileCoord coord);
TileCoord GetCoordinate(const Map * map, int index);
box_t GetVisibleRegion(const Game * game);
bool IsInBounds(const Map * map, int x, int y);
bool LineOfSight(Map * map, TileCoord t1, TileCoord t2);
void RenderMap(const Game * game);
void CalculateDistances(Map * map, TileCoord coord, int ignore_flags);
bool ManhattenPathsAreClear(Map * map, int x0, int y0, int x1, int y1);
void FreeDistanceMapQueue(void);
bool TileIsAdjacentTo(const Map * map, TileCoord coord, TileType type, int num_directions);
void RenderTilesInRegion(const Game * game, const box_t * region, int tile_size, vec2_t offset, bool debug);
vec2_t GetRenderLocation(const Game * game, vec2_t pt);


#endif /* map_h */
