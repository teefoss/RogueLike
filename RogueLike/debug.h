//
//  debug.h
//  RogueLike
//
//  Created by Thomas Foster on 11/7/22.
//

#ifndef debug_h
#define debug_h

#include "video.h"

#define DEBUG_PRINT(...) \
do { \
    if ( show_debug_info ) { \
        V_SetGray(248); \
        V_PrintString(0, debug_row++ * V_CharHeight(), __VA_ARGS__); \
    } \
} while ( 0 );

extern int debug_row;
extern bool show_debug_info;
extern float frame_msec;
extern float update_msec;
extern float render_msec;
extern float tiles_msec;
extern float max_frame_msec;
extern bool show_debug_map;
extern bool show_distances;

bool TilesAreLitThatShouldntBe(Map * map);
void PrintTilesAreFucked(Map * map, const char * string);
void PrintTilesAreDarkThatShouldntBe(Map * map, const char * string);
void DebugWaitForKeyPress(void);

#endif /* debug_h */
