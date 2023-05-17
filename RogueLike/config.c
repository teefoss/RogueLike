//
//  config.c
//  RogueLike
//
//  Created by Thomas Foster on 5/17/23.
//

#include "config.h"
#include "genlib.h"

#define CONFIG_FILE "config.cfg"

int     cfg_fullscreen      = 0; // TODO: default is 1 in release version
float   cfg_window_scale    = 1.0f;

// String config example:
// char cfg_string_config[CONFIG_STR_LEN] = "Default";

static Config configs[] = {
    { "FULLSCREEN",     &cfg_fullscreen,    CONFIG_DECIMAL },
    { "WINDOW_SCALE",   &cfg_window_scale,  CONFIG_FLOAT },
};

static int num_configs;


static Config * GetConfig(const char * name)
{
    for ( int i = 0; i < num_configs; i++ ) {
        if ( strcmp(name, configs[i].name) == 0 ) {
            return &configs[i];
        }
    }

    return NULL;
}


static void ReadConfigFile(FILE * file)
{
    char line[256] = { 0 };
    int line_num = 0;

    while ( fgets(line, sizeof(line), file) != NULL ) {
        ++line_num;

        char config_name[CONFIG_STR_LEN];

        float num_param;
        char str_param[256] = { 0 };
        bool is_string_param = false;

        if ( sscanf(line, "%s %f\n", config_name, &num_param) != 2 ) {
            is_string_param = true;
            if ( sscanf(line, "%s %s\n", config_name, str_param) != 2 ) {
                fprintf(stderr, "Error in "CONFIG_FILE" on line %d\n", line_num);
                return;
            }
        }

        Config * config = GetConfig(config_name);
        if ( config == NULL ) {
            fprintf(stderr, "Bad config parameter name on line %d\n", line_num);
            return;
        }

        if ( is_string_param ) {
            strncpy((char *)config->location, str_param, CONFIG_STR_LEN);
        } else if ( config->format == CONFIG_DECIMAL ) {
            *(int *)config->location = (int)num_param;
        } else if ( config->format == CONFIG_FLOAT ) {
            *(float *)config->location = num_param;
        } else {
            fprintf(stderr, "Error in "CONFIG_FILE" on line %d\n", line_num);
        }
    }
}

static void WriteConfigFile(FILE * file)
{
    for ( int i = 0; i < num_configs; i++ ) {
        StringToUpper(configs[i].name);

        int count = fprintf(file, "%s ", configs[i].name);
        while ( count++ < 20 ) {
            fprintf(file, " ");
        }

        switch ( configs[i].format ) {
            case CONFIG_DECIMAL:
                fprintf(file, "%d\n", *(int *)configs[i].location);
                break;
            case CONFIG_FLOAT:
                fprintf(file, "%g\n", *(float *)configs[i].location);
                break;
            case CONFIG_STRING:
                fprintf(file, "%s\n", (char *)configs[i].location);
                break;
            default:
                break;
        }
    }
}

void LoadConfigFile(void)
{
    num_configs = ARRAY_SIZE(configs);

    FILE * file = fopen("config.cfg", "r");
    if ( file ) {
        ReadConfigFile(file);
    }

    fclose(file);
}

void SaveConfigFile(void)
{
    FILE * file = fopen("config.cfg", "w");
    if ( file ) {
        WriteConfigFile(file);
    }

    fclose(file);
}
