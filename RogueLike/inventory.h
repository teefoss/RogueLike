//
//  inventory.h
//  RogueLike
//
//  Created by Thomas Foster on 4/15/23.
//

#ifndef inventory_h
#define inventory_h

#include "item.h"

typedef struct {
    int item_counts[NUM_ITEMS];
    int selected_item;
} Inventory;

#endif /* inventory_h */
