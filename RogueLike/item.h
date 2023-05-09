//
//  item.h
//  RogueLike
//
//  Created by Thomas Foster on 3/30/23.
//

#ifndef item_h
#define item_h

#include "icon.h"

#include <stdbool.h>

typedef enum item {
    ITEM_NONE = -1,
    ITEM_HEALTH,    // +1 health
    ITEM_TURN,      // +1 turn
    ITEM_STRENGTH,  // +1 damage for one turn?
    ITEM_FUEL_SMALL,
    ITEM_FUEL_BIG,

//    ITEM_SHIELD,
//    ITEM_WOODEN_SWORD,

    // Stone of returning: drop and activate to return to stone.
    // Gem that allows you to go back a level (starting pad turns purple)
    // Charge: move continuously until hit something
    // hover and examine monster's health and attack rating
    // reveal all thingy - all tiles visible

    NUM_ITEMS,
} Item;

int ItemInfoWidth(Item item);
const char * ItemName(Item item);
Icon ItemIcon(Item item);

#endif /* item_h */
