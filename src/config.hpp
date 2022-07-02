/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstdint>

union ButtonAxisID
{
    struct 
    {
        int id           : 8;
        bool is_axis     : 1;
        bool is_positive : 1;
        bool is_mapped   : 1;
    };
    int16_t value;
};

enum ConfigType
{
    CONFIG_INT,
    CONFIG_FLOAT,
    CONFIG_MAPPING,
};

struct ButtonAxisMapping
{
    ButtonAxisID primary;
    ButtonAxisID secondary;
};

struct ControllerConfig 
{
    float deadzone;
    float outer_edge;
    unsigned int range;
    int is_clamped;

    float a2d_threshold;
    float a2d_trig;

    ButtonAxisMapping a;
    ButtonAxisMapping b;
    ButtonAxisMapping z;
    ButtonAxisMapping l;
    ButtonAxisMapping r;
    ButtonAxisMapping start;

    ButtonAxisMapping cup;
    ButtonAxisMapping cdown;
    ButtonAxisMapping cleft;
    ButtonAxisMapping cright;

    ButtonAxisMapping dup;
    ButtonAxisMapping ddown;
    ButtonAxisMapping dleft;
    ButtonAxisMapping dright;

    ButtonAxisMapping up;
    ButtonAxisMapping down;
    ButtonAxisMapping left;
    ButtonAxisMapping right;
};

struct ControllerConfigInfo
{
    enum ConfigType type;
    char property[64];
    int struct_offset;
};

extern ControllerConfig concfg;

extern char configpath[];

void config_load();
void config_save();
void config_initialize();
void config_deinit();
