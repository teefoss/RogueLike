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

#include "shorttypes.h"
#include "vector.h"

#include <stdbool.h>

#define MAX_ACTORS (128 * 128)

#define FOR_EACH_ACTOR(it, list) \
    for (  Actor * it = list.head; it != NULL; it = it->next )

#define FOR_EACH_ACTOR_CONST(it, list) \
    for ( const Actor * it = list.head; it != NULL; it = it->next )


typedef enum {
    ACTOR_NONE = -1,
    ACTOR_PLAYER,
    ACTOR_TORCH,
    ACTOR_BLOB,
    ACTOR_SPIDER,
    ACTOR_SUPER_SPIDER,
    ACTOR_GHOST,
    ACTOR_ITEM_HEALTH,
    ACTOR_ITEM_TURN,
    ACTOR_ITEM_STRENGTH,
    ACTOR_GOLD_KEY,
    ACTOR_OLD_KEY,
    ACTOR_BLOCK,
    ACTOR_VASE,
    ACTOR_CLOSED_CHEST,
    ACTOR_OPEN_CHEST,
    ACTOR_SWORD,
    ACTOR_PILLAR,
    ACTOR_WELL,
    ACTOR_SHACK_CLOSED,
    ACTOR_SHACK_OPEN,
    ACTOR_ITEM_FUEL_SMALL,
    ACTOR_ITEM_FUEL_BIG,
    ACTOR_ROPE,

    NUM_ACTOR_TYPES
} ActorType;


// Animated sprite frames are layed out horizontally.
// `cell` is the first frame in an animation.
typedef struct actor_sprite {
    struct { u8 x, y; } cell;
    u8 height;
    s8 y_offset;
    u8 num_frames;
    u16 frame_msec;
    u8 draw_priority;
} ActorSprite;


typedef struct {
    s8 health;
    u8 damage;
} ActorsStats;


typedef struct game Game;
typedef struct world World;
typedef struct actor Actor;

typedef struct {
    const char * name;
    ActorSprite sprite;
    u8 max_health;
    u8 damage;
    u8 light_radius;
    u8 light; // An actor's light propogates to surrounding tiles.
    Item item; // Actor is collectible, its associated item.
    u8 particle_color_palette_index;
    const char * attack_sound;
    const char * kill_message; // What you see when this actor kills you.

    struct {
        bool directional        : 1; // Look at 'facing_left' for sprite flip.
        bool takes_damage       : 1; // Can be hurt by other actors.
        bool blocks_sight       : 1;
        bool blocks_monsters    : 1;
        bool blocks_player      : 1;
        bool collectible        : 1; // Actors can walk through (no bump anim).
        bool floats             : 1; // Hovers in the air.
        bool no_shadow          : 1;
        bool no_draw_offset     : 1;
        bool moves_diagonally   : 1;
    } flags;

    void (* contact)(Actor * self, Actor * other);
    void (* contacted)(Actor * self, Actor * other); // When hit by something else.
    void (* action)(Actor *);

} ActorInfo;


struct actor {
    Game * game;
    const ActorInfo * info;
    u8 type;

    struct {
        bool facing_left        : 1;
        bool was_attacked       : 1;
        bool has_target         : 1;
        bool on_teleporter      : 1; // TODO: move to PlayerInfo
    } flags;

    TileCoord tile;

    vec2_t offset_start;
    vec2_t offset_current;

    u8 frame;

    ActorsStats stats;
    TileCoord target_tile;
    float hit_timer;

    void (* animation)(Actor *, float move_timer);

    Actor * prev;
    Actor * next;
};

extern const ActorInfo actor_info_list[NUM_ACTOR_TYPES];

bool ActorBlocksAll(const Actor * actor);
/// Propogate actor's light to surrounding tiles by setting their `light_target`
/// value.
void CastLight(World * world, const Actor * actor);
void SetActorType(Actor * actor, ActorType type);
Actor * SpawnActor(Game * game, ActorType type, TileCoord coord);
void RenderActor(const Actor * actor, int x, int y, int size, bool debug, int game_ticks);
void MoveActor(Actor * actor, TileCoord coord);
bool TryMoveActor(Actor * actor, TileCoord coord);
int DamageActor(Actor * actor, Actor * inflictor, int damage);
void KillActor(Actor * actor, Actor * killer);
void UpdateActorFacing(Actor * actor, int dx);
void Teleport(Actor * actor);

/// Remove actor from linked list and return it to the free list.
///
/// When in a loop, if removing then immediately adding an actor, make sure
/// to add the new actor first, then remove the old one. That way, the current
/// pointer will remain pointing to the same actor (now in the free list).
void RemoveActor(Actor * actor);

Actor ** GetVisibleActors(const World * world,
                          const RenderInfo * render_info,
                          int * count);
void FreeVisibleActorsArray(void);

#endif /* actor_h */
