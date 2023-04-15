//
//  game.h
//  RogueLike
//
//  Created by Thomas Foster on 11/4/22.
//

#ifndef main_h
#define main_h

#include "vector.h"
#include "inttypes.h"
#include "coord.h"
#include "direction.h"
#include "tile.h"
#include "actor.h"
#include "item.h"
#include "area.h"
#include "particle.h"

#include "mathlib.h"

#include <SDL_rect.h>
#include <SDL_events.h>
#include <stdbool.h>

#define FPS 30.0f
#define DRAW_SCALE 4
#define GAME_WIDTH (256 * DRAW_SCALE)  // 32 tile wide
#define GAME_HEIGHT (144 * DRAW_SCALE) // 18 tiles high
//#define GAME_WIDTH (160 * DRAW_SCALE)  // 20 tile wide
//#define GAME_HEIGHT (100 * DRAW_SCALE) // 12.5 tiles high
#define TILE_SIZE 8
#define ICON_SIZE 6
#define SCALED(size) (size * DRAW_SCALE)

#define HUD_MARGIN 16
//#define INVENTORY_X ((float)GAME_WIDTH * 0.75f)

// Must be an odd number!
//#define MAP_WIDTH 31
//#define MAP_HEIGHT 31
#define MAP_MAX 100

#define DEBUG_TILE_SIZE 16
#define MAX_ROOMS 200

#define INITIAL_TURNS 0

#define DIR_BIT(direction) (1 << direction)
#define FLAG(i) (1 << i)
#define HAS_FLAG(flags, flag) (flags & FLAG(flag) != 0)

#define NUM_STARS 5000


#pragma mark - GAME


typedef struct {
    int count;
    Actor list[MAX_ACTORS];
} Actors;

typedef struct {
    int width;
    int height;
    Tile * tiles;
    TileID * tile_ids;
    int num_rooms;
    SDL_Rect rooms[MAX_ROOMS];
    Actors actors;
} Map;

typedef struct game Game;

typedef struct game_state {
    bool (* process_input)(Game *, const SDL_Event *);
    void (* update)(Game *, float dt);
    void (* render)(const Game *);

    void (* on_enter)(Game *);
    void (* on_exit)(Game *);

    // If state has a finite length, next_state must not be NULL.
    // -1 == indefinite length.
    int duration_ticks;
    const struct game_state * next_state;
} GameState;


typedef struct {
    int item_counts[NUM_ITEMS];
    int selected_item;
} Inventory;


typedef struct {
    SDL_Point pt;
    SDL_Color color;
} Star;

struct game {
    bool is_running;
    int ticks;
    Map map;
    float move_timer;
    TileCoord mouse_tile;

    vec2_t camera; // world focus point scaled coordinates
    float inventory_x;
    bool inventory_open;

    // Player
    Inventory inventory;
    bool has_gold_key;
    int player_turns;
    Area area;
    int level;
    
    // Dungeon
    int gold_key_room_num;

    // Forest
    Star stars[NUM_STARS];

    char log[100];

    int state_timer;
    const GameState * state; // TODO: stack

    ParticleArray particles;
};

bool InventoryIsEmtpy(const Inventory * inventory);
Game * InitGame(void);
void DoFrame(Game * game, float dt);
SDL_Rect GetLevelViewport(const Game * game);
vec2_t GetWindowScale(void);


#pragma mark - animation.c

void LevelTurnUpdate(Game * game, float dt);
void AnimateActorMove(Actor * actor, float move_timer);
void SetUpMoveAnimation(Actor * actor, Direction direction);
void SetUpBumpAnimation(Actor * actor, Direction direction);


#pragma mark - debug.c

extern bool show_map_gen;

void DebugWaitForKeyPress(void);
void CheckForShowMapGenCancel(void);


#pragma mark - map.c
// TODO: sep file

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

#pragma mark - player.c

#define GetPlayer(game) _Generic((game),    \
    const Game *: GetPlayerConst,         \
    Game *: GetPlayerNonConst             \
)(game)

void PlayerCastSight(Game * game);
bool CollectItem(Actor * player, Actor * item_actor, Item item);
const Actor * GetPlayerConst(const Game * game);
Actor * GetPlayerNonConst(Game * game);

#endif /* main_h */
