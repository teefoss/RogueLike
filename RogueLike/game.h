//
//  game.h
//  RogueLike
//
//  Created by Thomas Foster on 11/4/22.
//

#ifndef main_h
#define main_h

#include "vector.h"
#include "shorttypes.h"
#include "coord.h"
#include "direction.h"
#include "tile.h"
#include "actor.h"
#include "item.h"
#include "area.h"
#include "particle.h"
#include "world.h"
#include "render.h"
#include "inventory.h"
#include "game_state.h"

#include <SDL_rect.h>
#include <SDL_events.h>
#include <stdbool.h>

#define FPS 30.0f
#define MAP_MAX 100
#define INITIAL_TURNS 0

#define FLAG(x) (1 << x)

#define SOUND_BUMP "l32o0de-"

#define STEPS_PER_FUEL_UNIT 20
#define MAX_FUEL (STEPS_PER_FUEL_UNIT * 10)

typedef struct {
    Inventory inventory;
    bool has_gold_key;
    int player_turns;
    int strength_buff;
    int fuel;
} PlayerInfo;


typedef struct {
    Fade type;
    float timer;
    float duration_sec;
} FadeState;


struct game {
    bool is_running;
    int ticks;
    float move_timer;
    bool inventory_open;

    RenderInfo render_info;
    PlayerInfo player_info;
    int level;

    char log[100];

    int state_timer;
    int state_stack_top;
    const GameState * state_stack[MAX_GAME_STATES];

    // Used when changing states. The new state is stored here while a
    // fade out happens. At end of the fade, game state is changed this.
    const GameState * post_fade_state;

    FadeState fade_state;

    World world;
};

bool InventoryIsEmtpy(const Inventory * inventory);
Game * InitGame(void);
void DoFrame(Game * game, float dt);
vec2_t GetWindowScale(void);
void NewGame(Game * game);


#pragma mark - animation.c

void LevelTurnUpdate(Game * game, float dt);
void AnimateActorMove(Actor * actor, float move_timer);
void SetUpMoveAnimation(Actor * actor, Direction direction);
void SetUpBumpAnimation(Actor * actor, Direction direction);


#pragma mark - debug.c

extern bool show_map_gen;

void DebugWaitForKeyPress(void);
void CheckForShowMapGenCancel(void);


#pragma mark - player.c


void PlayerCastSight(World * world, const RenderInfo * render_info);
bool AddToInventory(Actor * player, Actor * item_actor, Item item);


#endif /* main_h */
