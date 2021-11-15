#define INI_IMPLEMENTATION
#include "ini.h"
#include "config.h"

ControllerConfig concfg;

void config_initialize(ControllerConfig *cfg)
{
    // default values
    cfg->deadzone = 0.05;
    cfg->range = 80;
    cfg->outer_edge = 0.95;
    cfg->is_clamped = 0;
}