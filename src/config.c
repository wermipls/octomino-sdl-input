#define INI_IMPLEMENTATION
#include "ini.h"
#include "config.h"
#include <stdio.h>
#include "sdl_input.h"

ControllerConfig concfg;
char configpath[PATH_MAX] = "Config\\" PLUGIN_NAME ".ini";

static const char suffix_primary[] = "_primary";
static const char suffix_secondary[] = "_secondary";

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
    return sprintf(dest, "%f", f);
}

static void set_property(ini_t *ini, int section_n, const char property[], char prop_val[])
{
    int prop_n = ini_find_property(ini, section_n, property, 0);
    if (prop_n == INI_NOT_FOUND) {
        ini_property_add(ini, section_n, property, 0, prop_val, 0);
        dlog("INI: property %s not found, initializing with %s", property, prop_val);
    } else {
        ini_property_value_set(ini, section_n, prop_n, prop_val, 0);
        dlog("INI: property %s found, setting value to %s", property, prop_val);
    }
}

static void set_property_float(ini_t *ini, int section_n, const char property[], float val)
{
    char prop_val[64];
    float2str(val, prop_val);

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
        dlog("INI: section %s not found", section);
        section_n = ini_section_add(ini, section, 0);
    }

    // read properties
    cfg->deadzone = read_property_float(ini, section_n, "deadzone", cfg->deadzone);
    cfg->outer_edge = read_property_float(ini, section_n, "outer_edge", cfg->outer_edge);
    cfg->range = read_property_int(ini, section_n, "range", cfg->range);
    cfg->is_clamped = read_property_int(ini, section_n, "is_clamped", cfg->is_clamped);

    // mapping cfg
    read_property_mapping(ini, section_n, "a", &cfg->a);
    read_property_mapping(ini, section_n, "b", &cfg->b);
    read_property_mapping(ini, section_n, "z", &cfg->z);
    read_property_mapping(ini, section_n, "l", &cfg->l);
    read_property_mapping(ini, section_n, "r", &cfg->r);
    read_property_mapping(ini, section_n, "start", &cfg->start);
    read_property_mapping(ini, section_n, "dup", &cfg->dup);
    read_property_mapping(ini, section_n, "ddown", &cfg->ddown);
    read_property_mapping(ini, section_n, "dleft", &cfg->dleft);
    read_property_mapping(ini, section_n, "dright", &cfg->dright);
    read_property_mapping(ini, section_n, "cup", &cfg->cup);
    read_property_mapping(ini, section_n, "cdown", &cfg->cdown);
    read_property_mapping(ini, section_n, "cleft", &cfg->cleft);
    read_property_mapping(ini, section_n, "cright", &cfg->cright);
    read_property_mapping(ini, section_n, "up", &cfg->up);
    read_property_mapping(ini, section_n, "down", &cfg->down);
    read_property_mapping(ini, section_n, "left", &cfg->left);
    read_property_mapping(ini, section_n, "right", &cfg->right);

    return;
}


static void config_save_con(ControllerConfig *cfg, ini_t *ini, char con_id)
{
    // find section
    char section[] = {"controller_0"};
    section[strlen(section) - 1] = con_id;

    int section_n = ini_find_section(ini, section, 0);
    if (section_n == INI_NOT_FOUND) {
        dlog("INI: section %s not found", section);
        section_n = ini_section_add(ini, section, 0);
    }

    // save properties
    set_property_float(ini, section_n, "deadzone", cfg->deadzone);
    set_property_float(ini, section_n, "outer_edge", cfg->outer_edge);
    set_property_int(ini, section_n, "range", cfg->range);
    set_property_int(ini, section_n, "is_clamped", cfg->is_clamped);
    // mapping cfg
    set_property_mapping(ini, section_n, "a", &cfg->a);
    set_property_mapping(ini, section_n, "b", &cfg->b);
    set_property_mapping(ini, section_n, "z", &cfg->z);
    set_property_mapping(ini, section_n, "l", &cfg->l);
    set_property_mapping(ini, section_n, "r", &cfg->r);
    set_property_mapping(ini, section_n, "start", &cfg->start);
    set_property_mapping(ini, section_n, "dup", &cfg->dup);
    set_property_mapping(ini, section_n, "ddown", &cfg->ddown);
    set_property_mapping(ini, section_n, "dleft", &cfg->dleft);
    set_property_mapping(ini, section_n, "dright", &cfg->dright);
    set_property_mapping(ini, section_n, "cup", &cfg->cup);
    set_property_mapping(ini, section_n, "cdown", &cfg->cdown);
    set_property_mapping(ini, section_n, "cleft", &cfg->cleft);
    set_property_mapping(ini, section_n, "cright", &cfg->cright);
    set_property_mapping(ini, section_n, "up", &cfg->up);
    set_property_mapping(ini, section_n, "down", &cfg->down);
    set_property_mapping(ini, section_n, "left", &cfg->left);
    set_property_mapping(ini, section_n, "right", &cfg->right);

    return;
}

void config_load()
{
    FILE *configfile = fopen(configpath, "rb");
    if (configini != NULL) {
        ini_destroy(configini);
    }
    configini = ini_load_file(configfile);
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
    fwrite(data, 1, size, configfile);
    fclose(configfile);

    free(data);
}

void config_initialize()
{
    // default values
    concfg.deadzone = 0.05;
    concfg.range = 80;
    concfg.outer_edge = 0.95;
    concfg.is_clamped = 0;

    // default controls
    concfg.a.primary        = CONTROLLER_A;
    concfg.a.secondary      = CONTROLLER_B;

    concfg.b.primary        = CONTROLLER_X;
    concfg.b.secondary      = CONTROLLER_Y;

    concfg.z.primary        = CONTROLLER_LTRIG;
    concfg.l.primary        = CONTROLLER_LSHOULDER;
    concfg.r.primary        = CONTROLLER_RTRIG;
    concfg.r.secondary      = CONTROLLER_RSHOULDER;

    concfg.start.primary    = CONTROLLER_START;

    concfg.dup.primary      = CONTROLLER_DUP;
    concfg.ddown.primary    = CONTROLLER_DDOWN;
    concfg.dleft.primary    = CONTROLLER_DLEFT;
    concfg.dright.primary   = CONTROLLER_DRIGHT;

    concfg.cup.primary      = CONTROLLER_RIGHTY_MIN;
    concfg.cdown.primary    = CONTROLLER_RIGHTY;
    concfg.cleft.primary    = CONTROLLER_RIGHTX_MIN;
    concfg.cright.primary   = CONTROLLER_RIGHTX;

    concfg.up.primary       = CONTROLLER_LEFTY_MIN;
    concfg.down.primary     = CONTROLLER_LEFTY;
    concfg.left.primary     = CONTROLLER_LEFTX_MIN;
    concfg.right.primary    = CONTROLLER_LEFTX;

    config_load();
}
