//
//  debug.c
//  RogueLike
//
//  Created by Thomas Foster on 11/4/22.
//

#include "game.h"
#include "debug.h"

int debug_row;
bool show_debug_info;
bool show_map_gen = false;

void DebugWaitForKeyPress(void)
{
    const u8 * keys = SDL_GetKeyboardState(NULL);
    while ( !keys[SDL_SCANCODE_ESCAPE] ) {
        SDL_PumpEvents();
        SDL_Delay(10);
    }
}

void CheckForShowMapGenCancel(void)
{
    SDL_PumpEvents();
    const u8 * keys = SDL_GetKeyboardState(NULL);
    if ( keys[SDL_SCANCODE_ESCAPE] ) {
        show_map_gen = false;
    }
}
