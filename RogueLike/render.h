//
//  render_info.h
//  RogueLike
//
//  Created by Thomas Foster on 4/15/23.
//

#ifndef render_h
#define render_h

#include "vector.h"
#include <SDL.h>

#define DRAW_SCALE 4

//#define GAME_WIDTH (256 * DRAW_SCALE)  // 32 tile wide
//#define GAME_HEIGHT (144 * DRAW_SCALE) // 18 tiles high
//#define GAME_HEIGHT (160 * DRAW_SCALE) // 18 tiles high
//#define TILES_WIDE (GAME_WIDTH / SCALED(TILE_SIZE)) // 32
//#define TILES_HIGH (GAME_HEIGHT / SCALED(TILE_SIZE)) // 18

#define HUD_MARGIN 16

#define TILE_SIZE 8
#define ICON_SIZE 6
#define DEBUG_TILE_SIZE 16

#define SCALED(size) ((size) * DRAW_SCALE)

typedef enum {
    NO_COLOR,
    GOLINE_DARK_PURPLE,
    GOLINE_PURPLE,
    GOLINE_RED,
    GOLINE_ORANGE,
    GOLINE_YELLOW,
    GOLINE_LIGHT_GREEN,
    GOLINE_DARK_GREEN,
    GOLINE_DARK_BLUE,
    GOLINE_BLACK,
    GOLINE_BLUE,
    GOLINE_LIGHT_BLUE,
    GOLINE_CYAN,
    GOLINE_WHITE,
    GOLINE_LIGHT_GRAY,
    GOLINE_BROWN,
    GOLINE_DARK_GRAY
} PaletteColor;

typedef struct {
    int width;
    int height;

    vec2_t camera; // world focus point in scaled coordinates
    float inventory_x; // the left side of the inventory panel

    SDL_Texture * stars;
    SDL_Texture * actor_texture;
    SDL_Texture * tile_texture;
    SDL_Texture * icon_texture;
} RenderInfo;

extern const SDL_Color palette[];

SDL_Rect GetLevelViewport(const RenderInfo * render_info);
vec2_t GetRenderLocation(const RenderInfo * render_info, vec2_t point);

/// Get the render offset according to the current position of the camera.
vec2_t GetRenderOffset(const RenderInfo * render_info);

void SetColor(PaletteColor color);

RenderInfo InitRenderInfo(int width, int height);
void FreeRenderAssets(RenderInfo * info);

SDL_Rect GetRenderSize(void);

/// Get how many tiles wide and tiles high the render area is.
SDL_Rect GetRenderSizeInTiles(void);

#endif /* render_h */
