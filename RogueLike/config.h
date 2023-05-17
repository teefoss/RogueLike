//
//  config.h
//  RogueLike
//
//  Created by Thomas Foster on 5/17/23.
//

#ifndef config_h
#define config_h

#define CONFIG_STR_LEN 80

typedef struct {
    char name[CONFIG_STR_LEN];
    void * location;
    enum {
        CONFIG_DECIMAL,
        CONFIG_FLOAT,
        CONFIG_STRING,
    } format;
} Config;

extern int     cfg_fullscreen;
extern float   cfg_window_scale;

void LoadConfigFile(void);
void SaveConfigFile(void);

#endif /* config_h */
