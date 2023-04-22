//
//  actor.h
//  RogueLike
//
//  Created by Thomas Foster on 3/29/23.
//

#ifndef actor_h
#define actor_h

#include "coord.h"
#include "direction.h"
#include "inventory.h"

#include "inttypes.h"
#include "vector.h"

#include <stdbool.h>

#define MAX_ACTORS (256 * 256)

typedef enum {
    ACTOR_PLAYER,
    ACTOR_TORCH,
    ACTOR_BLOB,
    ACTOR_SPIDER,
    ACTOR_ITEM_HEALTH,
    ACTOR_ITEM_TURN,
    ACTOR_GOLD_KEY,
    ACTOR_BLOCK,
    ACTOR_VASE,
    ACTOR_CLOSED_CHEST,
    ACTOR_OPEN_CHEST,
    ACTOR_SWORD,
    ACTOR_BLOCK_UP,
    ACTOR_BLOCK_DOWN,
    ACTOR_BUTTON_UP,
    ACTOR_BUTTON_DOWN,
    ACTOR_TREE,
    ACTOR_WELL,

    NUM_ACTOR_TYPES
} ActorType;


typedef struct actor_sprite {
    struct { u8 x, y; } cell;
    u8 height;
    s8 y_offset;
    u8 num_frames;
    u16 frame_msec;
    u8 draw_priority;
} ActorSprite;


typedef struct game Game;
typedef struct world World;
typedef struct actor Actor;

struct actor {
    Game * game;
    u8 type;

    struct {
        bool directional      : 1; // Look at 'facing_left' for sprite flip.
        bool takes_damage     : 1; // Can be hurt by other actors.
        bool was_attacked     : 1;
        bool has_target       : 1;
        bool blocks_sight     : 1;
        bool no_collision     : 1;
        bool collectible      : 1; // Actors can walk through (no bump anim).
        bool floats           : 1; // Hovers in the air.
        bool facing_left      : 1;
        bool no_shadow        : 1;
        bool no_draw_offset   : 1;
        bool on_teleporter    : 1;
        bool remove           : 1; // Deleted
    } flags;

    TileCoord tile;

    vec2_t offset_start;
    vec2_t offset_current;

    u8 frame;
    ActorSprite * sprite;

//    u8 num_frames; // TODO: in sprite_info
//    u16 frame_msec; // TODO: in sprite_info
//    u8 draw_priority; // TODO: in actor.c static const array

    u8 max_health;
    s8 health;
    u8 damage;
    TileCoord target_tile;

    // An actor's light propogates to surrounding tiles.
    u8 light;
    u8 light_radius;

    float hit_timer;

    void (* animation)(Actor *, float move_timer);
    void (* contact)(Actor * self, Actor * other);
    void (* contacted)(Actor * self, Actor * other); // When hit by something else.
    void (* action)(Actor *);
};


typedef struct {
    int count;
    Actor list[MAX_ACTORS];
} Actors;


/// Propogate actor's light to surrounding tiles by setting their `light_target`
/// value.
void CastLight(World * world, const Actor * actor);
void SpawnActor(Game * game, ActorType type, TileCoord coord);
void RenderActor(const Actor * actor, int x, int y, int size, bool debug, int game_ticks);
void MoveActor(Actor * actor, Direction direction);
bool TryMoveActor(Actor * actor, Direction direction);
int DamageActor(Actor * actor);
void KillActor(Actor * actor);
Actor * GetActorAtTile(Actors * actors, TileCoord coord);
const Actor * GetPlayerReadOnly(const Actor * actors, int num_actors);
void UpdateActorFacing(Actor * actor, int dx);
void Teleport(Actor * actor, TileCoord from);

#endif /* actor_h */
