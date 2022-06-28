/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define INI_IMPLEMENTATION
#include "ini.h"
#include "config.h"
#include <stdio.h>
#include <errno.h>
#include "sdl_input.h"

ControllerConfig concfg;
char configpath[PATH_MAX] = "Config\\" PLUGIN_NAME ".ini";

static const char suffix_primary[] = "_primary";
static const char suffix_secondary[] = "_secondary";

ini_t *configini;

static const ControllerConfigInfo concfg_field_info[] = {
    { CONFIG_FLOAT,   "deadzone",      offsetof(ControllerConfig, deadzone) },
    { CONFIG_FLOAT,   "outer_edge",    offsetof(ControllerConfig, outer_edge) },
    { CONFIG_INT,     "range",         offsetof(ControllerConfig, range) },
    { CONFIG_INT,     "is_clamped",    offsetof(ControllerConfig, is_clamped) },

    { CONFIG_FLOAT,   "a2d_threshold", offsetof(ControllerConfig, a2d_threshold) },
    { CONFIG_FLOAT,   "a2d_trig",      offsetof(ControllerConfig, a2d_trig) },

    { CONFIG_MAPPING, "a",             offsetof(ControllerConfig, a) },
    { CONFIG_MAPPING, "b",             offsetof(ControllerConfig, b) },
    { CONFIG_MAPPING, "z",             offsetof(ControllerConfig, z) },
    { CONFIG_MAPPING, "l",             offsetof(ControllerConfig, l) },
    { CONFIG_MAPPING, "r",             offsetof(ControllerConfig, r) },
    { CONFIG_MAPPING, "start",         offsetof(ControllerConfig, start) },
    { CONFIG_MAPPING, "dup",           offsetof(ControllerConfig, dup) },
    { CONFIG_MAPPING, "ddown",         offsetof(ControllerConfig, ddown) },
    { CONFIG_MAPPING, "dleft",         offsetof(ControllerConfig, dleft) },
    { CONFIG_MAPPING, "dright",        offsetof(ControllerConfig, dright) },
    { CONFIG_MAPPING, "cup",           offsetof(ControllerConfig, cup) },
    { CONFIG_MAPPING, "cdown",         offsetof(ControllerConfig, cdown) },
    { CONFIG_MAPPING, "cleft",         offsetof(ControllerConfig, cleft) },
    { CONFIG_MAPPING, "cright",        offsetof(ControllerConfig, cright) },
    { CONFIG_MAPPING, "up",            offsetof(ControllerConfig, up) },
    { CONFIG_MAPPING, "down",          offsetof(ControllerConfig, down) },
    { CONFIG_MAPPING, "left",          offsetof(ControllerConfig, left) },
    { CONFIG_MAPPING, "right",         offsetof(ControllerConfig, right) },
};

static const int concfg_field_count = sizeof(concfg_field_info) / sizeof(concfg_field_info[0]);

static void concfg_set_defaults(ControllerConfig *cfg)
{
    // default values
    cfg->deadzone = 0.05;
    cfg->range = 80;
    cfg->outer_edge = 0.95;
    cfg->is_clamped = 0;

    cfg->a2d_threshold = 0.25;
    cfg->a2d_trig = 0.25;

    // default controls
    cfg->a.primary        = CONTROLLER_A;
    cfg->a.secondary      = CONTROLLER_B;

    cfg->b.primary        = CONTROLLER_X;
    cfg->b.secondary      = CONTROLLER_Y;

    cfg->z.primary        = CONTROLLER_LTRIG;
    cfg->l.primary        = CONTROLLER_LSHOULDER;
    cfg->r.primary        = CONTROLLER_RTRIG;
    cfg->r.secondary      = CONTROLLER_RSHOULDER;

    cfg->start.primary    = CONTROLLER_START;

    cfg->dup.primary      = CONTROLLER_DUP;
    cfg->ddown.primary    = CONTROLLER_DDOWN;
    cfg->dleft.primary    = CONTROLLER_DLEFT;
    cfg->dright.primary   = CONTROLLER_DRIGHT;

    cfg->cup.primary      = CONTROLLER_RIGHTY_MIN;
    cfg->cdown.primary    = CONTROLLER_RIGHTY;
    cfg->cleft.primary    = CONTROLLER_RIGHTX_MIN;
    cfg->cright.primary   = CONTROLLER_RIGHTX;

    cfg->up.primary       = CONTROLLER_LEFTY_MIN;
    cfg->down.primary     = CONTROLLER_LEFTY;
    cfg->left.primary     = CONTROLLER_LEFTX_MIN;
    cfg->right.primary    = CONTROLLER_LEFTX;
}

static ini_t *ini_load_file(FILE *f)
{
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);

    ini_t *ini;

    if (size > 0) {
        char *data = (char*) malloc(size + 1);
        fread(data, 1, size, f);
        data[size] = '\0';

        fclose(f);

        ini = ini_load(data, NULL);

        free(data);
    } else {
        ini = ini_create(0);
    }

    return ini;
}

static int float2str(float f, char *dest, int size)
{
    return snprintf(dest, size, "%f", f);
}

static void set_property(ini_t *ini, int section_n, const char property[], char prop_val[])
{
    int prop_n = ini_find_property(ini, section_n, property, 0);
    if (prop_n == INI_NOT_FOUND) {
        ini_property_add(ini, section_n, property, 0, prop_val, 0);
    } else {
        ini_property_value_set(ini, section_n, prop_n, prop_val, 0);
    }
}

static void set_property_float(ini_t *ini, int section_n, const char property[], float val)
{
    char prop_val[64];
    float2str(val, prop_val, sizeof(prop_val));

    set_property(ini, section_n, property, prop_val);
}

static void set_property_int(ini_t *ini, int section_n, const char property[], int val)
{
    char prop_val[64];
    itoa(val, prop_val, 10);

    set_property(ini, section_n, property, prop_val);
}

static void set_property_mapping(ini_t *ini, int section_n, const char property[], ControllerMapping *val)
{
    char property_buf[64];

    strncpy(property_buf, property, sizeof(property_buf));
    strncat(property_buf, suffix_primary, sizeof(property_buf)-1);

    set_property_int(ini, section_n, property_buf, val->primary);

    strncpy(property_buf, property, sizeof(property_buf));
    strncat(property_buf, suffix_secondary, sizeof(property_buf)-1);

    set_property_int(ini, section_n, property_buf, val->secondary);
}

static float read_property_float(ini_t *ini, int section_n, const char property[], float defaultval)
{
    int prop_n = ini_find_property(ini, section_n, property, 0);
    if (prop_n == INI_NOT_FOUND) {
        char prop_val[64];
        float2str(defaultval, prop_val, sizeof(prop_val));
        ini_property_add(ini, section_n, property, 0, prop_val, 0);
        return defaultval;
    } else {
        const char *prop_val = ini_property_value(ini, section_n, prop_n);
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
        return defaultval;
    } else {
        const char *prop_val = ini_property_value(ini, section_n, prop_n);
        return atoi(prop_val);
    }
}

static void read_property_mapping(ini_t *ini, int section_n, const char property[], ControllerMapping *val)
{
    char property_buf[64];

    strncpy(property_buf, property, sizeof(property_buf));
    strncat(property_buf, suffix_primary, sizeof(property_buf)-1);

    val->primary = read_property_int(ini, section_n, property_buf, val->primary);

    strncpy(property_buf, property, sizeof(property_buf));
    strncat(property_buf, suffix_secondary, sizeof(property_buf)-1);

    val->secondary = read_property_int(ini, section_n, property_buf, val->secondary);
}

static void config_load_con(ControllerConfig *cfg, ini_t *ini, char con_id)
{
    // find section
    char section[] = {"controller_0"};
    section[strlen(section) - 1] = con_id;

    int section_n = ini_find_section(ini, section, 0);
    if (section_n == INI_NOT_FOUND) {
        section_n = ini_section_add(ini, section, 0);
    }

    // read properties
    for (int i = 0; i < concfg_field_count; ++i) {
        ControllerConfigInfo field = concfg_field_info[i];

        void *p = (void*)cfg + field.struct_offset;

        int *val_i = p;
        float *val_f = p;
        ControllerMapping *val_m = p;

        switch (field.type) 
        {
            case CONFIG_INT:
                *val_i = read_property_int(ini, section_n, field.property, *val_i);
                break;
            case CONFIG_FLOAT:
                *val_f = read_property_float(ini, section_n, field.property, *val_f);
                break;
            case CONFIG_MAPPING:
                read_property_mapping(ini, section_n, field.property, val_m);
                break;
        }
    }

    return;
}


static void config_save_con(ControllerConfig *cfg, ini_t *ini, char con_id)
{
    // find section
    char section[] = {"controller_0"};
    section[strlen(section) - 1] = con_id;

    int section_n = ini_find_section(ini, section, 0);
    if (section_n == INI_NOT_FOUND) {
        section_n = ini_section_add(ini, section, 0);
    }

    // save properties
    for (int i = 0; i < concfg_field_count; ++i) {
        ControllerConfigInfo field = concfg_field_info[i];

        void *p = (void*)cfg + field.struct_offset;

        int *val_i = p;
        float *val_f = p;
        ControllerMapping *val_m = p;

        switch (field.type) 
        {
            case CONFIG_INT:
                set_property_int(ini, section_n, field.property, *val_i);
                break;
            case CONFIG_FLOAT:
                set_property_float(ini, section_n, field.property, *val_f);
                break;
            case CONFIG_MAPPING:
                set_property_mapping(ini, section_n, field.property, val_m);
                break;
        }
    }

    return;
}

void config_load()
{
    FILE *configfile = fopen(configpath, "rb");
    if (configini != NULL) {
        ini_destroy(configini);
    }
    if (configfile != NULL) {
        configini = ini_load_file(configfile);
        dlog("Loaded config file %s", configpath);
    } else {
        configini = ini_create(0);
        dlog("Unable to open config file %s: %s", configpath, strerror(errno));
    }
    fclose(configfile);

    config_load_con(&concfg, configini, '0');
}

void config_save()
{
    if (configini == NULL) {
        config_initialize();
    } else {
        config_save_con(&concfg, configini, '0');
    }

    int size = ini_save(configini, NULL, 0);
    char *data = (char*) malloc(size);
    size = ini_save(configini, data, size);

    FILE *configfile = fopen(configpath, "wb");
    if (configfile == NULL) {
        dlog("Unable to save config file %s: %s", configpath, strerror(errno));
    } else {
        fwrite(data, 1, size, configfile);
        fclose(configfile);
        dlog("Saved config file %s", configpath);
    }

    free(data);
}

void config_initialize()
{
    concfg_set_defaults(&concfg);
    config_load();
}

void config_deinit()
{
    ini_destroy(configini);
}
