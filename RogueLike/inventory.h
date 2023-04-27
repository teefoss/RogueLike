//
//  inventory.h
//  RogueLike
//
//  Created by Thomas Foster on 4/15/23.
//

#ifndef inventory_h
#define inventory_h

#include "item.h"
#include "render.h"

typedef struct {
    int item_counts[NUM_ITEMS];
    int selected_item;
} Inventory;

bool InventoryIsEmtpy(const Inventory * inventory);
void ChangeInventorySelection(Inventory * inventory, int direction);
void RenderInventory(const Inventory * inv, const RenderInfo * info);
int InventoryWidth(void);

#endif /* inventory_h */
