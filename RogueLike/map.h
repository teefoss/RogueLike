//
//  map.h
//  RogueLike
//
//  Created by Thomas Foster on 4/14/23.
//

#ifndef map_h
#define map_h

#include "tile.h"
#include "render.h"
#include "coord.h"
#include "direction.h"
#include "actor_list.h"

#include "mathlib.h"

#define MAX_ROOMS 64

typedef struct {
    int width;
    int height;

    ActorList actor_list;
    Tile * tiles;
    TileID * tile_ids;

    int num_rooms;
    SDL_Rect rooms[MAX_ROOMS];
    int gold_key_room_num;
} Map;

Tile * GetAdjacentTile(Map * map, TileCoord coord, Direction direction);

TileCoord GetCoordinate(const Map * map, int index);
Box GetCameraVisibleRegion(const Map * map, const RenderInfo * render_info);
Box GetPlayerVisibleRegion(const Map * map, TileCoord player_coord);
bool IsInBounds(const Map * map, int x, int y);
bool LineOfSight(Map * map, TileCoord t1, TileCoord t2);
void CalculateDistances(Map * map, TileCoord coord, int ignore_flags, bool player);
bool ManhattenPathsAreClear(Map * map, int x0, int y0, int x1, int y1);
void FreeDistanceMapQueue(void);
bool TileIsAdjacentTo(const Map * map, TileCoord coord, TileType type, int num_directions);
int CalculateWallSignature(const Map * map, TileCoord coord, bool ignore_reveal);

#define GetTile(map, coord) _Generic((map), \
    const Map *: GetTileConst,              \
    Map *: GetTileNonConst                  \
)(map, coord)

Tile * GetTileNonConst(Map * map, TileCoord coord);
const Tile * GetTileConst(const Map * map, TileCoord coord);

#endif /* map_h */
