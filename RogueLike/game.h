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

#define DEBUG_TILE_SIZE 16
#define MAX_ROOMS 200
#define MAX_ACTORS 200

#define INITIAL_TURNS 0

#define DIR_BIT(direction) (1 << direction)
#define FLAG(i) (1 << i)
#define HAS_FLAG(flags, flag) (flags & FLAG(flag) != 0)


typedef struct {
    SDL_Point min; // upper left
    SDL_Point max; // lower right
} box_t;


#pragma mark - GAME


typedef struct {
    int count;
    actor_t list[MAX_ACTORS];

    // if targets[12] = 1: actor[12] is targeting actor[1]
    // if targetted[1] = 12: actor[1] is being targetted by actor[12]
    int targets[MAX_ACTORS];
    int targetted[MAX_ACTORS];
} actors_t;

typedef struct {
    int width;
    int height;
    tile_t * tiles;
    tile_id_t * tile_ids;
    int num_rooms;
    SDL_Rect rooms[MAX_ROOMS];
    actors_t actors;
} map_t;

typedef struct game game_t;

typedef struct game_state {
    bool (* process_input)(game_t *, const SDL_Event *);
    void (* update)(game_t *, float dt);
    void (* render)(const game_t *);

    void (* on_enter)(game_t *);
    void (* on_exit)(game_t *);

    // If state has a finite length, next_state must not be NULL.
    // -1 == indefinite length.
    int duration_ticks;
    const struct game_state * next_state;
} game_state_t;


typedef struct {
    int item_counts[NUM_ITEMS];
    int selected_item;
} inventory_t;


struct game {
    bool is_running;
    int ticks;
    map_t map;
    float move_timer;
    tile_coord_t mouse_tile;

    vec2_t camera;
    float inventory_x;
    bool inventory_open;

    // Player
    inventory_t inventory;
    bool has_gold_key;
    int player_turns;
    int level;

    int gold_key_room_num;

    char log[100];

    int state_timer;
    const game_state_t * state; // TODO: stack
};

bool InventoryIsEmtpy(const inventory_t * inventory);
game_t * InitGame(void);
void DoFrame(game_t * game, float dt);
SDL_Rect GetLevelViewport(const game_t * game);


#pragma mark - animation.c

void LevelTurnUpdate(game_t * game, float dt);
void AnimateActorMove(actor_t * actor, float move_timer);
void SetUpMoveAnimation(actor_t * actor, direction_t direction);
void SetUpBumpAnimation(actor_t * actor, direction_t direction);


#pragma mark - debug.c

extern bool show_map_gen;

void DebugWaitForKeyPress(void);
void CheckForShowMapGenCancel(void);


#pragma mark - gen.c

void GenerateDungeon(game_t * game, int width, int height);
void DebugRenderTiles(const map_t * map); // TODO: move these


#pragma mark - map.c

tile_t * GetAdjacentTile(map_t * map, tile_coord_t coord, direction_t direction);
tile_t * GetTile(map_t * map, tile_coord_t coord);
tile_coord_t GetCoordinate(const map_t * map, int index);
box_t GetVisibleRegion(const game_t * game);
bool IsInBounds(const map_t * map, int x, int y);
bool LineOfSight(map_t * map, tile_coord_t t1, tile_coord_t t2, bool reveal);
void RenderMap(const game_t * game);
void UpdateDistanceMap(map_t * map, tile_coord_t coord, int ignore_flags);
bool ManhattenPathsAreClear(map_t * map, int x0, int y0, int x1, int y1);
vec2_t GetRenderOffset(const actor_t * player);
void FreeDistanceMapQueue(void);
bool TileIsAdjacentTo(const map_t * map, tile_coord_t coord, tile_type_t type, int num_directions);

#pragma mark - player.c

#define GetPlayer(game) _Generic((game),    \
    const game_t *: GetPlayerConst,         \
    game_t *: GetPlayerNonConst             \
)(game)

void PlayerCastSightLines(game_t * game);
void CollectItem(actor_t * player, actor_t * item_actor, item_t item);
const actor_t * GetPlayerConst(const game_t * game);
actor_t * GetPlayerNonConst(game_t * game);

#endif /* main_h */
