//
//  icon.h
//  RogueLike
//
//  Created by Thomas Foster on 3/29/23.
//

#ifndef icon_h
#define icon_h

#include "inttypes.h"

#define ICON_SIZE 6

typedef enum {
    ICON_FULL_HEART,
    ICON_EMPTY_HEART,
    ICON_TURN,
    ICON_DOWN_ARROW,
    ICON_DAMAGE,
    ICON_GOLD_KEY,
    ICON_HEALTH_POTION,
    ICON_TURN_POTION,
    NUM_ICONS,
} icon_t;

void RenderIcon(icon_t icon, int x, int y);

#endif /* icon_h */
