/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>

#define CONTROLLER_AXIS_BEGIN CONTROLLER_LEFTX

enum ButtonAxis
{
    CONTROLLER_NOT_SET,

    CONTROLLER_A,
    CONTROLLER_B,
    CONTROLLER_X,
    CONTROLLER_Y,
    CONTROLLER_BACK,
    CONTROLLER_GUIDE,
    CONTROLLER_START,
    CONTROLLER_LSTICK,
    CONTROLLER_RSTICK,
    CONTROLLER_LSHOULDER,
    CONTROLLER_RSHOULDER,
    CONTROLLER_DUP,
    CONTROLLER_DDOWN,
    CONTROLLER_DLEFT,
    CONTROLLER_DRIGHT,

    CONTROLLER_LEFTX,
    CONTROLLER_LEFTY,
    CONTROLLER_RIGHTX,
    CONTROLLER_RIGHTY,
    CONTROLLER_LEFTX_MIN,
    CONTROLLER_LEFTY_MIN,
    CONTROLLER_RIGHTX_MIN,
    CONTROLLER_RIGHTY_MIN,
    CONTROLLER_LTRIG,
    CONTROLLER_RTRIG,

    CONTROLLER_ENUM_END,
};

enum ConfigType
{
    CONFIG_INT,
    CONFIG_FLOAT,
    CONFIG_MAPPING,
};

typedef struct ControlllerMapping
{
    enum ButtonAxis primary;
    enum ButtonAxis secondary;
} ControllerMapping;

typedef struct ControllerConfig 
{
    float deadzone;
    float outer_edge;
    unsigned int range;
    int is_clamped;

    float a2d_threshold;
    float a2d_trig;

    ControllerMapping a;
    ControllerMapping b;
    ControllerMapping z;
    ControllerMapping l;
    ControllerMapping r;
    ControllerMapping start;

    ControllerMapping cup;
    ControllerMapping cdown;
    ControllerMapping cleft;
    ControllerMapping cright;

    ControllerMapping dup;
    ControllerMapping ddown;
    ControllerMapping dleft;
    ControllerMapping dright;

    ControllerMapping up;
    ControllerMapping down;
    ControllerMapping left;
    ControllerMapping right;
} ControllerConfig;

typedef struct ControllerConfigInfo
{
    enum ConfigType type;
    char property[64];
    int struct_offset;
} ControllerConfigInfo;

extern ControllerConfig concfg;

extern char configpath[];

void config_load();
void config_save();
void config_initialize();
void config_deinit();
