//
//  contact.c
//  GameBeta
//
//  Created by Thomas Foster on 11/8/22.
//

#include "main.h"

void C_Player(actor_t * player, actor_t * hit)
{
    if ( hit->flags & ACTOR_FLAG_TAKES_DAMAGE ) {
        DamageActor(hit);
    }

    switch ( hit->type ) {
        case ACTOR_DOOR:
            hit->remove = true;
            break;
        default:
            break;
    }
}

void C_Monster(actor_t * monster, actor_t * hit)
{
    // Monsters can damage other monsters, but not of the same type
    if ( (hit->flags & ACTOR_FLAG_TAKES_DAMAGE) && monster->type != hit->type ) {
        DamageActor(hit);
    }
}
