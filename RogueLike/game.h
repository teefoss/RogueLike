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
#define RENDER_TILE_SIZE (TILE_SIZE * DRAW_SCALE)

// Must be an odd number!
#define MAP_WIDTH 31
#define MAP_HEIGHT 31

#define DEBUG_TILE_SIZE 16
#define MAX_ROOMS 200
#define MAX_ACTORS 200

#define INITIAL_TURNS 1

#define DIR_BIT(direction) (1 << direction)
#define FLAG(i) (1 << i)
#define HAS_FLAG(flags, flag) (flags & FLAG(flag) != 0)


typedef struct {
    SDL_Point min; // upper left
    SDL_Point max; // lower right
} box_t;


#pragma mark - ITEMS


typedef enum {
    ITEM_HEALTH,    // +1 health
    ITEM_TURN,      // +1 turn

    // Sword(s) wooden, steel, obsidian
    // Shield(s): 25% chance to block
    // - wooden 100% chance to break
    // - steel 75% chance to break
    // - obsidian 50% chance to break
    // Stone of returning: drop and activate to return to stone.
    // Gem that allows you to go back a level (starting pad turns purple)
    // Charge: move continuously until hit something
    // hover and examine monster's health and attack rating

    NUM_ITEMS,
} item_t;





#pragma mark - ACTOR

// When adding a new actor:
// ------------------------
// 1. Add to enum below, somewhere above 'NUM_ACTOR_TYPES'
// 2. Add a template in actor.c
// 3. Optionally, add a contact function in contact.c. Declare it at the
//    top of actor.c and add it to template.
// 4. Optionally, add an action function in action.c. Declare it at the
//    top of actior.c and add it to the template.
typedef enum {
    ACTOR_PLAYER,
    ACTOR_TORCH,
    ACTOR_BLOB,
    ACTOR_ITEM_HEALTH,
    ACTOR_ITEM_TURN,
    ACTOR_GOLD_KEY,

    NUM_ACTOR_TYPES
} actor_type_t;

typedef enum {
    // Actor can face left or right (sprite gets flipped per facing_left)
    ACTOR_DIRECTIONAL,
    ACTOR_TAKES_DAMAGE,
    ACTOR_BLOCKS_SIGHT,
    ACTOR_NO_BUMP, // Other actors can walk through.
    ACTOR_COLLECTIBLE, // Play can pick it up.
    ACTOR_FLOAT, // Hovers
} actor_flags_t;


typedef struct game game_t;
typedef struct actor actor_t;

struct actor {
    game_t * game;
    u8 type;

    struct __attribute__((packed)) {
        unsigned directional    : 1; // Look at 'facing_left' for sprite flip.
        unsigned takes_damage   : 1; // Can be hurt by other actors.
        unsigned blocks_sight   : 1;
        unsigned no_collision   : 1;
        unsigned collectible    : 1; // Actors can walk through (no bump anim).
        unsigned floats         : 1; // Hovers in the air.
        unsigned facing_left    : 1;
        unsigned was_attacked   : 1;
        unsigned remove         : 1; // Deleted
    } flags;

    tile_coord_t tile;

    vec2_t offset_start;
    vec2_t offset_current;

    u8 frame;
    u8 num_frames;
    u16 frame_msec;

    // Sprite sheet location.
    // Animated sprite frames are layed out horizontally.
    // sprite_location is the first frame in an animation.
    struct { u8 x, y; } sprite_cell;

    u8 max_health;
    s8 health;
    u8 damage;

    // An actor's light propogates to surrounding tiles.
    u8 light;
    u8 light_radius;

    float hit_timer;

    void (* animation)(actor_t *, float move_timer);
    void (* contact)(actor_t * self, actor_t * other);
    void (* action)(actor_t *);
};




#pragma mark - GAME

typedef struct {
    int count;
    actor_t list[MAX_ACTORS];
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


#pragma mark - actor.c

/// Propogate actor's light to surrounding tiles by setting their `light_target`
/// value.
void CastLight(game_t * game, const actor_t * actor);
void SpawnActor(game_t * game, actors_t * actors, actor_type_t type, tile_coord_t coord);
void RenderActor(const actor_t * actor, int offset_x, int offset_y);
void MoveActor(actor_t * actor, direction_t direction);
bool TryMoveActor(actor_t * actor, direction_t direction);
int DamageActor(actor_t * actor);
actor_t * GetActorAtXY(actor_t * actors, int num_actors, int x, int y);
actor_t * GetPlayer(actors_t * actors);
const actor_t * GetPlayerReadOnly(const actor_t * actors, int num_actors);
void UpdateActorFacing(actor_t * actor, int dx);

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
void DebugRenderActors(const actors_t * actors);


#pragma mark - map.c

tile_t * GetAdjacentTile(map_t * map, tile_coord_t coord, direction_t direction);
tile_t * GetTile(map_t * map, tile_coord_t coord);
tile_coord_t GetCoordinate(const map_t * map, int index);
box_t GetVisibleRegion(const map_t * map, const actor_t * player);
bool IsInBounds(const map_t * map, int x, int y);
bool LineOfSight(map_t * map, tile_coord_t t1, tile_coord_t t2, bool reveal);
void RenderMap(const game_t * game);
void UpdateDistanceMap(map_t * map, tile_coord_t coord, int ignore_flags);
bool ManhattenPathsAreClear(map_t * map, int x0, int y0, int x1, int y1);
vec2_t GetRenderOffset(const actor_t * player);
void FreeDistanceMapQueue(void);
bool TileIsAdjacentTo(const map_t * map, tile_coord_t coord, tile_type_t type, int num_directions);

///
/// Populate `out_array` with all tiles reachable from `coord`, given a
/// set of tile types to ignore (for example, doors).
/// - parameter ignore_flags: A bit field of `tile_type_t` flag values.
/// - returns: The number of tiles stored in `out_array`.
///
int GetReachableTiles(map_t * map, tile_coord_t coord, int ignore_flags, tile_coord_t * out_array);

#pragma mark - player.c

void PlayerCastSightLines(map_t * map);
void CollectItem(actor_t * player, actor_t * item_actor, item_t item);

#endif /* main_h */
