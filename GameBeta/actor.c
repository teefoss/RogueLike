//
//  actor.c
//  GameBeta
//
//  Created by Thomas Foster on 11/6/22.
//

#include "main.h"

#include "mathlib.h"
#include "texture.h"
#include "video.h"

void C_Player(actor_t * player, actor_t * hit);
void C_Monster(actor_t * monster, actor_t * hit);
void A_Blob(actor_t * blob, game_t * game);

static actor_t templates[NUM_ACTOR_TYPES] = {
    [ACTOR_PLAYER] = {
        .flags = ACTOR_FLAG_DIRECTIONAL | ACTOR_TAKES_DAMAGE,
        .num_frames = 2,
        .frame_msec = 200,
        .max_health = 3,
        .light = 255,
        .light_radius = 3,
        .contact = C_Player,
    },
    [ACTOR_TORCH] = {
        .num_frames = 2,
        .frame_msec = 300,
        .light = 255,
        .light_radius = 1,
    },
    [ACTOR_BLOB] = {
        .flags = ACTOR_TAKES_DAMAGE,
        .num_frames = 2,
        .frame_msec = 300,
        .max_health = 2,
        .action = A_Blob,
        .contact = C_Monster,
    },
};

void SpawnActor(game_t * game, actor_type_t type, int x, int y)
{
    actor_t actor = templates[type];
    actor.type = type;
    actor.x = x;
    actor.y = y;
    actor.health = actor.max_health;

    if ( game->num_actors + 1 <= MAX_ACTORS ) {
        game->actors[game->num_actors++] = actor;
    } else {
        printf("ran out of room in actor array!\n");
    }
}

void DamageActor(actor_t * actor)
{
    actor->hit_timer = 1.0f;
    if ( --actor->health == 0 ) {
        actor->remove = true;
    }
}

void RenderActor(const actor_t * actor, int offset_x, int offset_y)
{
    SDL_Texture * actor_sheet = GetTexture("assets/actors.png");

    SDL_Rect src;
    src.w = TILE_SIZE;
    src.h = TILE_SIZE;

    SDL_Rect dst;
    dst.x = (actor->x * RENDER_TILE_SIZE + actor->offset.x) - offset_x;
    dst.y = (actor->y * RENDER_TILE_SIZE + actor->offset.y) - offset_y - 2;
    dst.y -= 2 * DRAW_SCALE; // Place 2 pixels up on tiles.
    dst.w = RENDER_TILE_SIZE;
    dst.h = RENDER_TILE_SIZE;

    switch ( actor->type ) {
        case ACTOR_PLAYER:
            src.y = 0;
            break;
        case ACTOR_TORCH:
            src.y = 8;
            break;
        case ACTOR_BLOB:
            src.y = 16;
            break;
        case NUM_ACTOR_TYPES: // just shut the compiler up
            break;
        // Don't provide a default here so the compiler will warn if
        // we're missing an actor type.
    }

    src.x = TILE_SIZE * actor->frame;

    if ( actor->hit_timer > 0.0f ) {
        src.x += TILE_SIZE * actor->num_frames;
    }

    if ( (actor->flags & ACTOR_FLAG_DIRECTIONAL) && actor->facing_left ) {
        V_DrawTextureFlip(actor_sheet, &src, &dst, SDL_FLIP_HORIZONTAL);
    } else {
        V_DrawTexture(actor_sheet, &src, &dst);
    }
}

void CastLight(const actor_t * actor, tiles_t tiles)
{
    int r = actor->light_radius;

    if ( r == 0 ) {
        return;
    }

    for ( int y = actor->y - r; y <= actor->y + r; y++ ) {
        for ( int x = actor->x - r; x <= actor->x + r; x++ ) {
            tile_t * t = &tiles[y][x];

            if ( LineOfSight(tiles, actor->x, actor->y, x, y, false) ) {
                int distance = DISTANCE(actor->x, actor->y, x, y);

                if ( distance <= r) {
                    t->light_target = actor->light;
                }
            }
        }
    }
}

bool TryMoveActor(actor_t * actor, game_t * game, int dx, int dy)
{
    int try_x = actor->x + dx;
    int try_y = actor->y + dy;

    if ( game->map.tiles[try_y][try_x].type != TILE_FLOOR ) {
        return false;
    }

    // Check if there's an actor at try_x, try_y
    for ( int i = 0; i < game->num_actors; i++ ) {
        actor_t * hit = &game->actors[i];

        if ( hit != actor && hit->x == try_x && hit->y == try_y ) {
            // There's something here. Bump into it.
            SetUpBumpAnimation(actor, dx, dy);
            if ( actor->contact ) {
                actor->contact(actor, hit);
            }
            return false;
        }
    }

    actor->x = try_x;
    actor->y = try_y;

    if ( dx ) {
        actor->facing_left = dx < 0;
    }

    // Set up move animation
    actor->offset.x = -dx * RENDER_TILE_SIZE;
    actor->offset.y = -dy * RENDER_TILE_SIZE;
//    actor->offsets[1].x = 0;
//    actor->offsets[1].y = 0;
    actor->animation = AnimateActorMove;

    return true;
}
