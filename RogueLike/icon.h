//
//  icon.h
//  RogueLike
//
//  Created by Thomas Foster on 3/29/23.
//

#ifndef icon_h
#define icon_h

#include "shorttypes.h"
#include "render.h"

#define ICON_SIZE 6

typedef enum {
    ICON_HEART_FULL,
    ICON_HEART_EMPTY,
    ICON_FUEL_FULL,
    ICON_FUEL_EMPTY,
    ICON_FUEL_BURN,
    ICON_FUEL_DYING,
    ICON_FUEL_SMALL,
    ICON_FUEL_BIG,
    ICON_TURN,
    ICON_DOWN_ARROW,
    ICON_DAMAGE,
    ICON_GOLD_KEY,
    ICON_OLD_KEY,
    ICON_HEALTH_POTION,
    ICON_TURN_POTION,
    ICON_STRENGTH_POTION,
    NUM_ICONS,
} Icon;

void RenderIcon(Icon icon, int x, int y, const RenderInfo * render_info);

#endif /* icon_h */
