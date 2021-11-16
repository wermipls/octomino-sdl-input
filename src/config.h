#include <stdio.h>

typedef struct ControllerConfig 
{
    float deadzone;
    float outer_edge;
    unsigned int range;
    int is_clamped;
} ControllerConfig;

extern ControllerConfig concfg;

extern FILE *configfile;

void config_initialize(ControllerConfig *cfg);