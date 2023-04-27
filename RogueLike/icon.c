//
//  icon.c
//  RogueLike
//
//  Created by Thomas Foster on 3/29/23.
//

#include "icon.h"
#include "game.h"
#include "video.h"
#include "texture.h"

static struct {
    u8 x, y;
} info[NUM_ICONS] = {
    [ICON_HEART_FULL]       = { 0, 0 },
    [ICON_HEART_EMPTY]      = { 1, 0 },
    [ICON_TURN]             = { 2, 0 },
    [ICON_DOWN_ARROW]       = { 3, 0 },
    [ICON_DAMAGE]           = { 4, 0 },
    [ICON_GOLD_KEY]         = { 5, 0 },
    [ICON_HEALTH_POTION]    = { 0, 1 },
    [ICON_TURN_POTION]      = { 1, 1 },
    [ICON_STRENGTH_POTION]  = { 5, 1 },
    [ICON_FUEL_FULL]        = { 6, 0 },
    [ICON_FUEL_EMPTY]       = { 7, 0 },
};

static SDL_Rect src = { .w = ICON_SIZE, .h = ICON_SIZE };
static SDL_Rect dst = { .w = SCALED(ICON_SIZE), .h = SCALED(ICON_SIZE) };

void RenderIcon(Icon icon, int x, int y)
{
    SDL_Texture * icons_texture = GetTexture("assets/icons.png");

    src.x = info[icon].x * ICON_SIZE;
    src.y = info[icon].y * ICON_SIZE;
    dst.x = x;
    dst.y = y;

    V_DrawTexture(icons_texture, &src, &dst);
}
