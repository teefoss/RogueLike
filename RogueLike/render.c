//
//  render.c
//  RogueLike
//
//  Created by Thomas Foster on 4/15/23.
//

#include "render.h"
#include "game.h"
#include "video.h"

#include <SDL_image.h>

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


static void RenderMoon(int x, int y, int size)
{
    SDL_Rect moon1_rect = {
        .x = x - size / 2,
        .y = y - size / 2,
        .w = size,
        .h = size,
    };

    V_SetGray(248);
    V_FillRect(&moon1_rect);
}


static SDL_Texture * CreateForestBackgroundTexture(int width, int height)
{
    SDL_Texture * texture = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_RGBA8888,
                                              SDL_TEXTUREACCESS_TARGET,
                                              width,
                                              height);

    SDL_SetRenderTarget(renderer, texture);
    V_SetColor(area_info[AREA_FOREST].render_clear_color);
    V_Clear();

    for ( int i = 0; i < 5000; i++ ) {
        SDL_Point pt;
        pt.x = Random(0, width - 1);
        pt.y = Random(0, height - 1);

        int n = Random(0, 1000);
        if ( n == 1000 ) {
            V_SetRGB(0, 248, 0);
        } else if ( n < 500 ) {
            V_SetRGB(208, 208, 208);
        } else {
            V_SetRGB(128, 128, 128);
        }

        // Half a pixel wide
        SDL_Rect r = { pt.x, pt.y, SCALED(1) / 2, SCALED(1) / 2 };
        V_FillRect(&r);
    }

    int moom_size = SCALED(TILE_SIZE * 3);
    RenderMoon(width * 0.66, height * 0.66, moom_size);
    RenderMoon(width * 0.33, height * 0.33, moom_size * 0.66);

    SDL_SetRenderTarget(renderer, NULL);

    return texture;
}


static SDL_Texture * LoadTexture(const char * file)
{
    SDL_Texture * texture = NULL;
    SDL_Surface * surface = IMG_Load(file);

    if ( surface ) {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    if ( texture == NULL ) {
        printf("error: could not load %s\n!", file);
    }

    return texture;
}


#pragma mark -


/// Level area rect. Width and height are scaled.
SDL_Rect GetLevelViewport(const RenderInfo * render_info)
{
    int inventory_width = render_info->width - render_info->inventory_x;
    SDL_Rect viewport = { 0, 0, render_info->width - inventory_width, render_info->height };

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


RenderInfo InitRenderInfo(int width, int height)
{
    RenderInfo info = { 0 };

    info.width = width;
    info.height = height;

    info.inventory_x = width; // Start closed.

    info.stars = CreateForestBackgroundTexture(width, height);

    info.actor_texture = LoadTexture("assets/actors.png");
    info.tile_texture = LoadTexture("assets/tiles2.png");
    info.icon_texture = LoadTexture("assets/icons.png");

    return info;
}


void FreeRenderAssets(RenderInfo * info)
{
    SDL_DestroyTexture(info->stars);
    SDL_DestroyTexture(info->actor_texture);
    SDL_DestroyTexture(info->tile_texture);
    SDL_DestroyTexture(info->icon_texture);
}


SDL_Rect GetRenderSize(void) 
{
    int window_w;
    int window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);
    float aspect = (float)window_h / (float)window_w;

    SDL_Rect size;
    size.h = 18 * SCALED(TILE_SIZE);
    size.w = (float)size.h / aspect;

    return size;
}

SDL_Rect GetRenderSizeInTiles(void)
{
    SDL_Rect size = GetRenderSize();
    size.w /= SCALED(TILE_SIZE);
    size.h /= SCALED(TILE_SIZE);

    return size;
}
