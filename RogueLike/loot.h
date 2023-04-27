//
//  loot.h
//  RogueLike
//
//  Created by Thomas Foster on 4/25/23.
//

#ifndef loot_h
#define loot_h

#include "actor.h"

typedef struct {
    ActorType actor_type;
    int weight;
} Loot;

extern const Loot blob_loot[];
ActorType SelectLoot(ActorType actor_type);

#endif /* loot_h */
