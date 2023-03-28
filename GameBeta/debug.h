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
        V_SetGray(200); \
        V_PrintString(0, debug_row++ * V_CharHeight(), __VA_ARGS__); \
    } \
} while ( 0 );

extern int debug_row;
extern bool show_debug_info;

void DebugWaitForKeyPress(void);

#endif /* debug_h */
