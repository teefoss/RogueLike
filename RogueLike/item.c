//
//  item.c
//  RogueLike
//
//  Created by Thomas Foster on 3/30/23.
//

#include "item.h"
#include "video.h"
#include "game.h"

static const struct {
    const char * name;
    icon_t icon;
} item_info[NUM_ITEMS] = {
    [ITEM_HEALTH] = {
        .name = "Health Potion",
        .icon = ICON_HEALTH_POTION,
    },
    [ITEM_TURN] = {
        .name = "Turn Potion",
        .icon = ICON_TURN_POTION,
    },
};


void RenderItemInfo(item_t item, int count, int x, int y, bool is_selected)
{
    V_SetRGB(255, 255, 255);
    int x1 = V_PrintString(x, y, "%d", count);

    x1 += SCALED(1);
    RenderIcon(item_info[item].icon, x1, y);

    x1 += SCALED(ICON_SIZE);
    if ( is_selected ) {
        V_SetRGB(255, 255, 0);
    }
    V_PrintString(x1, y, "%s", item_info[item].name);
}


int ItemInfoWidth(item_t item)
{
    int width = 0;
    width += V_CharWidth() + SCALED(1); // count
    width += SCALED(ICON_SIZE); // icon
    width += strlen(item_info[item].name) * V_CharWidth(); // name

    return width;
}
