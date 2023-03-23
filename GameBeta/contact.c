//
//  contact.c
//  GameBeta
//
//  Created by Thomas Foster on 11/8/22.
//

#include "main.h"
#include "mylib/sound.h"

void C_Player(actor_t * player, actor_t * hit)
{
    if ( hit->flags & FLAG(ACTOR_TAKES_DAMAGE) ) {
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
                    "Picked up a Health Potion.",
                    sizeof(player->game->log));
            S_Play("l32 o2 e b g+ > d+");
            break;
        case ACTOR_ITEM_TURN:
            CollectItem(player, hit, ITEM_TURN);
            strncpy(player->game->log,
                    "Picked up a Turn Potion.",
                    sizeof(player->game->log));
            S_Play("l32 o2 a b > f+");
            break;
        default:
            break;
    }
}

void C_Monster(actor_t * monster, actor_t * hit)
{
    // Monsters can damage other monsters, but not of the same type
    if ( (hit->flags & FLAG(ACTOR_TAKES_DAMAGE)) && monster->type != hit->type ) {
        DamageActor(hit);
    }
}
