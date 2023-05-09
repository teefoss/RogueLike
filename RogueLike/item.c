//
//  item.c
//  RogueLike
//
//  Created by Thomas Foster on 3/30/23.
//

#include "item.h"
#include "video.h"
#include "game.h"
#include "loot.h"

static const struct {
    const char * name;
    Icon icon;
} item_info[NUM_ITEMS] = {
    [ITEM_HEALTH] = {
        .name = "Health Potion",
        .icon = ICON_HEALTH_POTION,
    },
    [ITEM_TURN] = {
        .name = "Turn Potion",
        .icon = ICON_TURN_POTION,
    },
    [ITEM_STRENGTH] = {
        .name = "Strength Potion",
        .icon = ICON_STRENGTH_POTION,
    },
    [ITEM_FUEL_SMALL] = {
        .name = "Small Lamp Fuel",
        .icon = ICON_FUEL_SMALL,
    },
    [ITEM_FUEL_BIG] = {
        .name = "Large Lamp Fuel",
        .icon = ICON_FUEL_BIG,
    },
};


int ItemInfoWidth(Item item)
{
    int width = 0;
    width += V_CharWidth() + SCALED(1); // count
    width += SCALED(ICON_SIZE); // icon
    width += strlen(item_info[item].name) * V_CharWidth(); // name

    return width;
}


const char * ItemName(Item item)
{
    return item_info[item].name;
}


Icon ItemIcon(Item item)
{
    return item_info[item].icon;
}
