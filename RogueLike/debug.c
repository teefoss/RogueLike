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
bool show_debug_map;
bool show_distances;

float frame_msec;
float update_msec;
float render_msec;
float tiles_msec;
float actors_msec;
float max_frame_msec;


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


bool TilesAreLitThatShouldntBe(Map * map)
{
    for ( int i = 0; i < map->width * map->height; i++ ) {
        Tile * tile = &map->tiles[i];

        if ( !tile->flags.revealed && tile->light > 0 ) {
            return true;
        }
    }

    return false;
}


void PrintTilesAreFucked(Map * map, const char * string)
{
    bool effed = TilesAreLitThatShouldntBe(map);
    printf("%s: tiles %s\n", string, effed ? "effed" : "OK");
}


void PrintTilesAreDarkThatShouldntBe(Map * map, const char * string)
{
    for ( int y = 0; y < map->height; y++ ) {
        for (int x = 0; x < map->width; x++ ) {
            Tile * tile = GetTile(map, ((TileCoord){ x, y }));
            if ( tile->flags.revealed && tile->light == 0 ) {
                printf("%s: fucked\n", string);
                return;
            }
        }
    }

    printf("%s: OK\n", string);
}
