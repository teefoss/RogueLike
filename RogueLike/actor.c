//
//  actor.c
//  RogueLike
//
//  Created by Thomas Foster on 11/6/22.
//

#include "game.h"
#include "world.h"
#include "render.h"
#include "loot.h"
#include "actor_list.h"
#include "game_state.h"

#include "mathlib.h"
#include "texture.h"
#include "video.h"
#include "sound.h"

#include <limits.h>

enum {
    DRAW_PRIORITY_NONE,
    DRAW_PRIORITY_ITEM,
    DRAW_PRIORITY_KEY,
    DRAW_PRIORITY_MONSTER,
    DRAW_PRIORITY_PLAYER,
};

void C_Player(Actor *, Actor *);
void C_Monster(Actor *, Actor *);
void C_Block(Actor *, Actor *);

void A_TargetAndChasePlayerIfVisible(Actor *);
void A_ChasePlayerIfVisible(Actor *);
void A_StupidChasePlayerIfVisible(Actor *);
void A_SpiderChase(Actor *);
void A_GhostChase(Actor *);

#define ITEM_FLAGS { .collectible = true, .no_collision = true }

const ActorInfo actor_info_list[NUM_ACTOR_TYPES] = {
    [ACTOR_PLAYER] = {
        .name = "Player",
        .flags = { .directional = true, .takes_damage = true },
        .max_health = 10,
        .damage = 1,
        .light = 255,
        .light_radius = 3,
        .contact = C_Player,
        .sprite = {
            .cell = { 0, 0 },
            .num_frames = 2,
            .frame_msec = 500,
            .draw_priority = DRAW_PRIORITY_PLAYER,
        },
    },
    [ACTOR_TORCH] = {
        .name = "Torch",
        .light = 255,
        .light_radius = 2,
        .sprite = {
            .cell = { 2, 3 },
            .num_frames = 2,
            .frame_msec = 300,
        },
    },
    [ACTOR_BLOB] = {
        .name = "Blob",
        .flags = { .takes_damage = true, },
        .max_health = 2,
        .damage = 1,
        .action = A_TargetAndChasePlayerIfVisible,
        .contact = C_Monster,
        .light_radius = 1,
        .light = 160,
        .attack_sound = "o3 l32 b c < c+",
        .particle_color_palette_index = GOLINE_DARK_GREEN,
        .kill_message = "A Blob absorbs you.",
        .sprite = {
            .cell = { 0, 2 },
            .num_frames = 2,
            .frame_msec = 300,
            .draw_priority = DRAW_PRIORITY_MONSTER,
        },
    },
    [ACTOR_SPIDER] = {
        .name = "Spider",
        .flags = { .takes_damage = true, },
        .max_health = 1,
        .damage = 1,
        .action = A_SpiderChase,
        .contact = C_Monster,
        .attack_sound = "o4 l32 f b c+",
        .particle_color_palette_index = GOLINE_BLACK,
        .kill_message = "You have been devoured by a Spider!",
        .sprite = {
            .cell = { 4, 2 },
            .num_frames = 4,
            .frame_msec = 100,
            .draw_priority = DRAW_PRIORITY_MONSTER,
        },
    },
    [ACTOR_SUPER_SPIDER] = {
        .name = "Super Spider",
        .flags = { .takes_damage = true, },
        .max_health = 2,
        .damage = 2,
        .action = A_TargetAndChasePlayerIfVisible,
        .contact = C_Monster,
        .attack_sound = "o3 l32 f b c+",
        .particle_color_palette_index = GOLINE_PURPLE,
        .kill_message = "You have been consumed by a Super Spider!",
        .sprite = {
            .cell = { 4, 6 },
            .num_frames = 4,
            .frame_msec = 100,
            .draw_priority = DRAW_PRIORITY_MONSTER,
        },
    },
    [ACTOR_GHOST] = {
        .name = "Ghost",
        .flags = { .takes_damage = true, .floats = true },
        .max_health = 3,
        .damage = 3,
        .action = A_GhostChase,
        .contact = C_Monster,
        .attack_sound = "o1 l32 c e a- > d- f",
        .particle_color_palette_index = GOLINE_WHITE,
        .kill_message = "A Ghost ate your soul!",
        .sprite = {
            .cell = { 6, 1 },
            .num_frames = 1,
            .draw_priority = DRAW_PRIORITY_MONSTER,
        },
    },
    [ACTOR_ITEM_HEALTH] = {
        .name = "Health Potion",
        .item = ITEM_HEALTH,
        .flags = ITEM_FLAGS,
        .sprite = {
            .cell = { 0, 1 },
            .draw_priority = DRAW_PRIORITY_ITEM,
        },
    },
    [ACTOR_ITEM_TURN] = {
        .name = "Turn Potion",
        .item = ITEM_TURN,
        .flags = ITEM_FLAGS,
        .sprite = {
            .cell = { 1, 1 },
            .draw_priority = DRAW_PRIORITY_ITEM,
        },
    },
    [ACTOR_ITEM_STRENGTH] = {
        .name = "Strength Potion",
        .item = ITEM_STRENGTH,
        .flags = ITEM_FLAGS,
        .sprite = {
            .cell = { 3, 1 },
            .draw_priority = DRAW_PRIORITY_ITEM,
        },
    },
    [ACTOR_ITEM_FUEL_SMALL] = {
        .name = "Small Lamp Fuel",
        .item = ITEM_FUEL_SMALL,
        .flags = ITEM_FLAGS,
        .sprite = {
            .cell = { 4, 1 },
            .draw_priority = DRAW_PRIORITY_ITEM,
        },
    },
    [ACTOR_ITEM_FUEL_BIG] = {
        .name = "Large Lamp Fuel",
        .item = ITEM_FUEL_BIG,
        .flags = ITEM_FLAGS,
        .sprite = {
            .cell = { 5, 1 },
            .draw_priority = DRAW_PRIORITY_ITEM,
        },
    },
    [ACTOR_GOLD_KEY] = {
        .name = "Gold Key",
        .flags = ITEM_FLAGS,
        .sprite = {
            .cell = { 2, 1 },
            .draw_priority = DRAW_PRIORITY_KEY,
        },
    },
    [ACTOR_BLOCK] = {
        .name = "Block",
        .contacted = C_Block,
        .sprite = {
            .cell = { 4, 3 }
        },
    },
    [ACTOR_CLOSED_CHEST] = {
        .name = "Closed Chest",
        .flags = { .takes_damage = true, },
        .max_health = 1,
        .sprite = {
            .cell = { 0, 3 }
        },
    },
    [ACTOR_OPEN_CHEST] = {
        .name = "Chest",
        .flags = { .no_collision = true },
        .sprite = {
            .cell = { 1, 3 }
        },
    },
    [ACTOR_PILLAR] = {
        .name = "Pillar",
        .flags = { .no_shadow = true },
        .sprite = {
            .cell = { 0, 6 }
        },
    },
    [ACTOR_WELL] = {
        .name = "Well",
        .flags = { .no_shadow = true, .no_draw_offset = true },
        .sprite = {
            .cell = { 3, 6 },
            .height = 2,
            .y_offset = -1,
        },
    },
    [ACTOR_SHACK_CLOSED] = {
        .name = "Closed Shack",
        .flags = { .no_shadow = true, .no_draw_offset = true },
        .sprite = {
            .cell = { 1, 6 },
            .height = 2,
            .y_offset = -1,
        }
    },
    [ACTOR_SHACK_OPEN] = {
        .name = "Open Shack",
        .flags = { .no_shadow = true, .no_draw_offset = true },
        .sprite = {
            .cell = { 2, 6 },
            .height = 2,
            .y_offset = -1,
        }
    },
    [ACTOR_VASE] = {
        .name = "Vase",
        .flags = { .takes_damage = true, },
        .particle_color_palette_index = GOLINE_PURPLE,
        .max_health = 1,
        .sprite = {
            .cell = { 5, 0 }
        },
    }
};


void SetActorType(Actor * actor, ActorType type)
{
    actor->info = &actor_info_list[type];
    actor->type = type;

    // Reset stats on change.
    actor->stats.health = actor->info->max_health;
    actor->stats.damage = actor->info->damage;
}


Actor * SpawnActor(Game * game, ActorType type, TileCoord coord)
{
    Actor * actor;
    ActorList * list = &game->world.map->actor_list;

    // If there are actors available in the unused list, grab one from there.
    // Otherwise, allocate a new one.
    if ( list->free_list ) {
        actor = list->free_list;
        list->free_list = actor->prev;
    } else {
        actor = calloc(1, sizeof(*actor));

        if ( actor == NULL ) {
            Error("Could not allocate Actor");
        }
    }

    memset(actor, 0, sizeof(*actor));

    // Append to active list.
    if ( list->tail ) {
        list->tail->next = actor;
    }
    actor->prev = list->tail; // ->next is already NULL.
    list->tail = actor;

    if ( list->head == NULL ) {
        list->head = actor;
    }

    list->count++;

    // Init.

    actor->game = game;
    actor->tile = coord;
    SetActorType(actor, type);

    return actor;
}


void SpawnParticlesAtActor(const Actor * actor)
{
    SDL_Point position = {
        .x = actor->tile.x * TILE_SIZE,
        .y = actor->tile.y * TILE_SIZE,
    };

    for ( int i = 0; i < 30; i++ ) {
        Particle p;
        p.position.x = Random(position.x, position.x + TILE_SIZE);
        p.position.y = Random(position.y, position.y + TILE_SIZE);
        p.velocity = RandomVelocity(10.0f, 20.0f);
        p.color = palette[actor->info->particle_color_palette_index];
        p.lifespan = Random(MS2TICKS(200, FPS), MS2TICKS(400, FPS));
        InsertParticle(&actor->game->world.particles, p);
    }
}


void KillActor(Actor * actor, Actor * killer)
{
    ActorType loot = SelectLoot(actor->type);

    if ( loot != ACTOR_NONE ) {
        SpawnActor(actor->game, loot, actor->tile);
    }

    if ( actor->info->particle_color_palette_index != NO_COLOR ) {
        SpawnParticlesAtActor(actor);
    }

    // Remove the actor after loot have been spawned, so that loot does not
    // replace this actor before it's fully processed.

    switch ( actor->type ) {
        case ACTOR_PLAYER: {
            actor->game->kill_message = killer->info->kill_message;
            ChangeState(actor->game, &gs_death_screen);
            break;
        }
        case ACTOR_CLOSED_CHEST:
            SpawnActor(actor->game, ACTOR_OPEN_CHEST, actor->tile);
            RemoveActor(actor);
            break;
        default:
            RemoveActor(actor);
            break;
    }
}


int DamageActor(Actor * actor, Actor * inflictor, int damage)
{
    actor->hit_timer = 1.0f;
    actor->flags.was_attacked = true;

    actor->stats.health -= damage;
    if ( actor->stats.health <= 0 ) {
        KillActor(actor, inflictor);
    }

    return actor->stats.health;
}


void RenderActor(const Actor * actor, int x, int y, int size, bool debug, int game_ticks)
{
    SDL_Texture * actor_sheet = actor->game->render_info.actor_texture;
    const ActorSprite * sprite = &actor->info->sprite;

    Tile * tile = GetTile(actor->game->world.map, actor->tile);
    if ( !debug ) {
        SDL_SetTextureColorMod(actor_sheet, tile->light, tile->light, tile->light);
    } else {
        SDL_SetTextureColorMod(actor_sheet, 255, 255, 255);
    }

    SDL_Rect src;
    src.x = (sprite->cell.x + actor->frame) * TILE_SIZE;
    src.y = sprite->cell.y * TILE_SIZE;
    src.w = TILE_SIZE;

    // "Damaged" sprite?
    if ( actor->hit_timer > 0.0f ) {
        src.x += TILE_SIZE * sprite->num_frames;
    }

    SDL_Rect dst;
    dst.x = x;
    dst.y = y + sprite->y_offset * size;
    dst.w = size;

    // Adjust height.
    if ( actor->info->sprite.height ) {
        src.h = sprite->height * TILE_SIZE;
        dst.h = sprite->height * size;
    } else {
        src.h = TILE_SIZE;
        dst.h = size;
    }

    // Draw actor's shadow
    // TODO: adjust shadow for tall sprites
    if ( !actor->info->flags.no_shadow ) {
        SDL_Rect shadow_sprite_location = {
            .x = 4 * TILE_SIZE,
            .y = 0 * TILE_SIZE,
            .w = TILE_SIZE,
            .h = TILE_SIZE
        };
        V_DrawTexture(actor_sheet, &shadow_sprite_location, &dst);
    }

    // y position tweaks
    // TODO: adjust for tall sprites
    if ( actor->info->flags.floats ) {
        dst.y += (sinf(game_ticks / 7) * SCALED(1)) - SCALED(6);
    } else {
        if ( !debug ) {
            dst.y -= SCALED(3);
        }
    }

    if ( actor->info->flags.directional && actor->flags.facing_left ) {
        V_DrawTextureFlip(actor_sheet, &src, &dst, SDL_FLIP_HORIZONTAL);
    } else {
        V_DrawTexture(actor_sheet, &src, &dst);
    }
}


void CastLight(World * world, const Actor * actor)
{
    int r = actor->info->light_radius;

    if ( r == 0 ) {
        return;
    }

    TileCoord coord;
    for ( coord.y = actor->tile.y - r; coord.y <= actor->tile.y + r; coord.y++ ) {
        for ( coord.x = actor->tile.x - r; coord.x <= actor->tile.x + r; coord.x++ ) {
            Tile * t = GetTile(world->map, coord);

//            if ( t && LineOfSight(&game->map, actor->tile, coord, false))

            if ( !t || !t->flags.visible ) {
                continue;
            }

            if ( t && ManhattenPathsAreClear(world->map,
                                             actor->tile.x,
                                             actor->tile.y,
                                             coord.x,
                                             coord.y) )
            {
                if ( TileDistance(actor->tile, coord) <= r ) {
                    if ( t->flags.revealed && actor->info->light > t->light ) {
                        t->light = actor->info->light;
                    }
                }
            }
        }
    }
}

// TODO: This needs to be Move to tile with a helper function move direction!
void MoveActor(Actor * actor, TileCoord coord)
{
    if ( !TileCoordsEqual(actor->tile, coord) ) {
        Direction direction = GetHorizontalDirection(coord.x - actor->tile.x);

        SetUpMoveAnimation(actor, coord);
        actor->tile = coord;
        UpdateActorFacing(actor, XDelta(direction));
    }

    if ( actor->type == ACTOR_PLAYER ) {
        World * world = &actor->game->world;

        ResetTileVisibility(world, actor->tile, &actor->game->render_info);
        PlayerCastSight(world, &actor->game->render_info);
    }
}


bool TryMoveActor(Actor * actor, TileCoord coord)
{
//    Tile * tile = GetAdjacentTile(&actor->game->world.map, actor->tile, direction);
    Tile * tile = GetTile(actor->game->world.map, coord);
//    TileCoord try_coord = AdjacentTileCoord(actor->tile, direction);
    TileCoord try_coord = coord;

    if ( tile->flags.blocks_movement ) {
        return false;
    }

    // Tile is player-only.
    if ( actor->type != ACTOR_PLAYER && tile->flags.player_only ) {
        return false;
    }

//    Direction direction = NO_DIRECTION;
    int dx = coord.x - actor->tile.x;
    int dy = coord.y - actor->tile.y;
    Direction direction = GetDirection(dx, dy);

    UpdateActorFacing(actor, dx);

    // Check if there's an actor at try_x, try_y
    FOR_EACH_ACTOR(hit, actor->game->world.map->actor_list) {

        if ( hit != actor && TileCoordsEqual(hit->tile, try_coord) ) {

            // There's an actor on this spot:

            if ( actor->info->contact ) {
                actor->info->contact(actor, hit);
            }

            if ( hit->info->contacted ) {
                hit->info->contacted(hit, actor);
            }

            // Bump into it?
            if ( !hit->info->flags.no_collision ) {
                SetUpBumpAnimation(actor, direction);
                return false;
            }
        }
    }

    MoveActor(actor, coord);
    return true;
}

void UpdateActorFacing(Actor * actor, int dx)
{
    if ( dx ) {
        actor->flags.facing_left = dx < 0;
    }
}


// TODO: telefrag?
void Teleport(Actor * actor)
{
    Map * map = actor->game->world.map;

    Tile * entry_tile = GetTile(map, actor->tile);
    int tag = entry_tile->tag;

    // Find the nearest teleport.
    for ( int i = 0; i < map->width * map->height; i++ ) {
        if (   map->tiles[i].type == TILE_TELEPORTER
            && map->tiles[i].tag == tag
            && &map->tiles[i] != entry_tile )
        {
            TileCoord exit_coord = GetCoordinate(map, i);

            actor->tile = exit_coord;
            MoveActor(actor, actor->tile); // TODO: hack, update sight etc.
            return;
        }
    }

    // No connected teleporter found? Don't do anything.
    printf("no teleporter with tag %d found\n", tag);
}


void RemoveActor(Actor * actor)
{
    ActorList * list = &actor->game->world.map->actor_list;

    // Remove from actor list.
    if ( actor == list->tail ) {
        list->tail = actor->prev;
    } else {
        actor->next->prev = actor->prev;
    }

    if ( actor == list->head ) {
        list->head = list->head->next;
    } else {
        actor->prev->next = actor->next;
    }

    // Add to unused list.
    actor->prev = list->free_list;
    list->free_list = actor;
}


// Include a padding since the camera may be mid-tile. For the y,
// include extra padding in case there are tall actors visible
static Actor ** visible_actors = NULL;
static int capacity = 0;

Actor ** GetVisibleActors(const World * world,
                          const RenderInfo * render_info,
                          int * count)
{
    Box vis_rect = GetCameraVisibleRegion(world->map, render_info);
    int w = (vis_rect.right - vis_rect.left) + 1;
    int h = (vis_rect.bottom - vis_rect.top) + 1;
    int area = w * h;

    // Resize array if needed;
    if ( area > capacity ) {
        printf("area %d is greater than capacity %d, resizing\n", area, capacity);
        size_t new_size = area * sizeof(Actor *);
        if ( capacity == 0 ) {
            visible_actors = malloc(new_size);
        } else {
            visible_actors = realloc(visible_actors, new_size);
        }

        if ( visible_actors == NULL ) {
            Error("could not malloc visible actor array");
        }

        capacity = area;
    }

    *count = 0;

    FOR_EACH_ACTOR_CONST(actor, world->map->actor_list) {
        const Tile * tile = GetTile(world->map, actor->tile);

        if ( tile->flags.visible && TileInBox(actor->tile, vis_rect) ) {
            visible_actors[(*count)++] = (Actor *)actor; // fuck it
        }
    }

    return visible_actors;
}


void FreeVisibleActorsArray(void)
{
    if ( visible_actors ) {
        free(visible_actors);
    }
}
