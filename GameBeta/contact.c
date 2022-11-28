//
//  contact.c
//  GameBeta
//
//  Created by Thomas Foster on 11/8/22.
//

#include "main.h"

void C_Player(actor_t * player, actor_t * hit)
{
    switch ( hit->type ) {
        case ACTOR_TORCH:
            hit->remove = true;
            break;
        default:
            break;
    }
}
