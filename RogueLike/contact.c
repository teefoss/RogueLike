//
//  contact.c
//  RogueLike
//
//  Created by Thomas Foster on 11/8/22.
//

#include "game.h"
#include "mylib/sound.h"

void C_Player(actor_t * player, actor_t * hit)
{
    if ( hit->flags.takes_damage ) {
        if ( DamageActor(hit) == 0 ) {
            S_Play("t160 l32 o3 e c+ c < a-");
        } else {
            S_Play("t160 l32 o3 e c+ c");
        }
    }

    switch ( hit->type ) {
        case ACTOR_ITEM_HEALTH:
            CollectItem(player, hit, ITEM_HEALTH);
            strncpy(player->game->log,
                    "Picked up a Health Potion!",
                    sizeof(player->game->log));
            S_Play("l32 o2 e b g+ > d+");
            break;
        case ACTOR_ITEM_TURN:
            CollectItem(player, hit, ITEM_TURN);
            strncpy(player->game->log,
                    "Picked up a Turn Potion!",
                    sizeof(player->game->log));
            S_Play("l32 o2 a b > f+");
            break;
        case ACTOR_GOLD_KEY:
            strncpy(player->game->log, "Got the golden key!", sizeof(player->game->log));
            player->game->has_gold_key = true;
            hit->flags.remove = true;
            S_Play("l32 t100 o1 a > a > a");
            break;
        case ACTOR_VASE:
            S_Play("o5 t90 l64 b d g+ e- c f b-");
            SpawnActor(player->game, ACTOR_ITEM_HEALTH, hit->tile);
            hit->flags.remove = true;
            break;
        default:
            break;
    }
}

void C_Monster(actor_t * monster, actor_t * hit)
{
    // Monsters can damage other monsters, but not of the same type
    if ( hit->flags.takes_damage && monster->type != hit->type ) {
        DamageActor(hit);
        S_Play("o3 l32 b c < c+");
    }
}

void C_Block(actor_t * block, actor_t * pusher)
{
    int dx = block->tile.x - pusher->tile.x;
    int dy = block->tile.y - pusher->tile.y;

    if ( TryMoveActor(block, GetDirection(dx, dy)) ) {
        S_Play("l32 o1 b-f-");
    }
}
