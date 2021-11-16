#include <stdio.h>

typedef struct ControllerConfig 
{
    float deadzone;
    float outer_edge;
    unsigned int range;
    int is_clamped;
} ControllerConfig;

extern ControllerConfig concfg;

extern char configpath[];

void config_load();
void config_save();
void config_initialize();