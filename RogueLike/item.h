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

typedef enum {
    ITEM_HEALTH,    // +1 health
    ITEM_TURN,      // +1 turn

    ITEM_SHIELD,
    ITEM_WOODEN_SWORD,

    // Stone of returning: drop and activate to return to stone.
    // Gem that allows you to go back a level (starting pad turns purple)
    // Charge: move continuously until hit something
    // hover and examine monster's health and attack rating

    NUM_ITEMS,
} Item;

void RenderItemInfo(Item item, int count, int x, int y, bool is_selected);
int ItemInfoWidth(Item item);

#endif /* item_h */
