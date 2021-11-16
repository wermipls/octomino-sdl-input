#define INI_IMPLEMENTATION
#include "ini.h"
#include "config.h"
#include <stdio.h>
#include "sdl_input.h"

ControllerConfig concfg;
FILE *configfile;

ini_t *configini;

static ini_t *ini_load_file(FILE *f)
{
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = (char*) malloc(size + 1);
    fread(data, 1, size, f);
    data[size] = '\0';

    fclose(f);

    ini_t *ini = ini_load(data, NULL);

    free(data);

    return ini;
}

static int float2str(float f, char *dest)
{
    return sprintf(dest, "%f", dest);
}

static float read_property_float(ini_t *ini, int section_n, const char property[], float defaultval)
{
    int prop_n = ini_find_property(ini, section_n, property, 0);
    if (prop_n == INI_NOT_FOUND) {
        char prop_val[64];
        float2str(defaultval, prop_val);
        ini_property_add(ini, section_n, property, 0, prop_val, 0);
        dlog("INI: property %s not found, initializing with default %s", property, prop_val);
        return defaultval;
    } else {
        const char *prop_val = ini_property_value(ini, section_n, prop_n);
        dlog("INI: property %s found, value: %s", property, prop_val);
        return atof(prop_val);
    }
}

static float read_property_int(ini_t *ini, int section_n, const char property[], int defaultval)
{
    int prop_n = ini_find_property(ini, section_n, property, 0);
    if (prop_n == INI_NOT_FOUND) {
        char prop_val[64];
        itoa(defaultval, prop_val, 10);
        ini_property_add(ini, section_n, property, 0, prop_val, 0);
        dlog("INI: property %s not found, initializing with default %s", property, prop_val);
        return defaultval;
    } else {
        const char *prop_val = ini_property_value(ini, section_n, prop_n);
        dlog("INI: property %s found, value: %s", property, prop_val);
        return atoi(prop_val);
    }
}

static void config_load_con(ControllerConfig *cfg, ini_t *ini, char con_id)
{
    // find section
    char section[] = {"controller_0"};
    //section[strlen(section)] = con_id;

    int section_n = ini_find_section(ini, section, 0);
    if (section_n == INI_NOT_FOUND) {
        dlog("INI: section %s not found", section);
        section_n = ini_section_add(ini, section, 0);
    }

    int prop_n;
    const char *prop_val;

    // read properties
    cfg->deadzone = read_property_float(ini, section_n, "deadzone", cfg->deadzone);
    cfg->outer_edge = read_property_float(ini, section_n, "outer_edge", cfg->outer_edge);
    cfg->range = read_property_int(ini, section_n, "range", cfg->range);
    cfg->is_clamped = read_property_int(ini, section_n, "is_clamped", cfg->is_clamped);

    return;
}

void config_initialize(ControllerConfig *cfg)
{
    // default values
    cfg->deadzone = 0.05;
    cfg->range = 80;
    cfg->outer_edge = 0.95;
    cfg->is_clamped = 0;

    // load file contents
    configini = ini_load_file(configfile);

    config_load_con(&concfg, configini, '0');
}