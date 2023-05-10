//
//  render.c
//  RogueLike
//
//  Created by Thomas Foster on 4/15/23.
//

#include "render.h"
#include "game.h"
#include "video.h"

const SDL_Color palette[] = {
    { 0, 0, 0, 0 }, // NO_COLOR
    { 67,   0, 103,    255 },
    { 148,  33, 106,    255 },
    { 255,   0,  77,    255 },
    { 255, 132,  38,    255 },
    { 255, 221,  52,    255 },
    { 80, 225,  18,    255 },
    { 63, 166, 111,    255 },
    { 54,  89, 135,    255 },
    { 0,   0,   0,    255 },
    { 0,  51, 255,    255 },
    { 41, 173, 255,    255 },
    { 0, 255, 204,    255 },
    { 255, 241, 232,    255 },
    { 194, 195, 199,    255 },
    { 171,  82,  54,    255 },
    { 95,  87,  79,    255 },
};

/// Level area rect. Width and height are scaled.
SDL_Rect GetLevelViewport(const RenderInfo * render_info)
{
    int inventory_width = GAME_WIDTH - render_info->inventory_x;
    SDL_Rect viewport = { 0, 0, GAME_WIDTH - inventory_width, GAME_HEIGHT };

    return viewport;
}


/// - parameter point: world scaled coordinates
vec2_t GetRenderLocation(const RenderInfo * render_info, vec2_t point)
{
    SDL_Rect viewport = GetLevelViewport(render_info);

    int half_w = (viewport.w - SCALED(TILE_SIZE)) / 2;
    int half_h = (viewport.h - SCALED(TILE_SIZE)) / 2;

    vec2_t offset;
    offset.x = point.x - half_w;
    offset.y = point.y - half_h;

    return offset;
}


vec2_t GetRenderOffset(const RenderInfo * render_info)
{
    return GetRenderLocation(render_info, render_info->camera);
}


void SetColor(PaletteColor color)
{
    V_SetColor(palette[color]);
}
