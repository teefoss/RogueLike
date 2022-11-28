//
//  debug.c
//  GameBeta
//
//  Created by Thomas Foster on 11/4/22.
//

#include "main.h"
#include "debug.h"

int debug_row;
bool show_debug_info;

void DebugWaitForKeyPress(void)
{
    while ( 1 ) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        if ( event.type == SDL_KEYDOWN ) {
            return;
        }
        SDL_Delay(1);
    }
}
