/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <math.h>
#include "zilmar_controller_1.0.h"
#include "sdl_input.h"
#include "gui.h"
#include "config.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        // make a log file
        CreateDirectoryA("Logs", NULL);
        logfile = fopen("Logs\\" PLUGIN_NAME ".txt", "w");

        // get path to gamecontroller.txt
        GetModuleFileNameA(hinstDLL, dbpath, sizeof(dbpath));
        PathRemoveFileSpecA(dbpath);
        PathCombineA(dbpath, dbpath, "gamecontrollerdb.txt");

        // make/load a config file
        CreateDirectoryA("Config", NULL);
        config_initialize(&concfg);

        break;
    case DLL_PROCESS_DETACH:
        fclose(logfile);
        break;
    }
    return TRUE;
}

EXPORT void CALL CloseDLL(void)
{
    deinit();
}

//EXPORT void CALL ControllerCommand(int Control, BYTE * Command) {}

EXPORT void CALL DllAbout(HWND hParent)
{
    MessageBoxA(
        hParent,
        PLUGIN_ABOUT,
        "About " PLUGIN_NAME " v" PLUGIN_VERSION,
        MB_ICONINFORMATION
    );
}

EXPORT void CALL DllConfig(HWND hParent)
{
    con_open();
    config_window();
}

//EXPORT void CALL DllTest(HWND hParent) {}

EXPORT void CALL GetDllInfo(PLUGIN_INFO * PluginInfo)
{
    PluginInfo->Version = 0x0100;
    PluginInfo->Type = PLUGIN_TYPE_CONTROLLER;
    snprintf(
        PluginInfo->Name,
        sizeof(PluginInfo->Name),
        "%s v%s",
        PLUGIN_NAME, PLUGIN_VERSION
    );
}

static inline void n64_analog(BUTTONS *Keys, int16_t x, int16_t y)
{
    x = ((int32_t)x * (int32_t)concfg.range) / 32767;
    y = ((int32_t)y * (int32_t)concfg.range) / 32767;

    if (concfg.is_clamped) {
        int16_t lim_x = 80 - (int16_t)round(abs(sclamp(y, -70, 70)) / 7. - 1. / 7);
        int16_t lim_y = 80 - (int16_t)round(abs(sclamp(x, -70, 70)) / 7. - 1. / 7);

        if (lim_x < x) {
            x = lim_x;
            y = y * (lim_x / x);
        }
        if (-lim_x > x) {
            x = -lim_x;
            y = y * (-lim_x / x);
        }
        if (lim_y < y) {
            y = lim_y;
            x = x * (lim_y / y);
        }
        if (-lim_y > y) {
            y = -lim_y;
            x = x * (-lim_y / y);
        }
    }

    Keys->Y_AXIS = x;
    Keys->X_AXIS = -y;
}

static int16_t get_state_buttonaxis(inputs_t *i, enum ButtonAxis ba)
{
    switch (ba)
    {
        case CONTROLLER_NOT_SET:
            return 0;
        case CONTROLLER_A:
            return i->a;
        case CONTROLLER_B:
            return i->b;
        case CONTROLLER_X:
            return i->x;
        case CONTROLLER_Y:
            return i->y;
        case CONTROLLER_BACK:
            return i->back;
        case CONTROLLER_GUIDE:
            return i->guide;
        case CONTROLLER_START:
            return i->start;
        case CONTROLLER_LSTICK:
            return i->lstick;
        case CONTROLLER_RSTICK:
            return i->rstick;
        case CONTROLLER_LSHOULDER:
            return i->lshoul;
        case CONTROLLER_RSHOULDER:
            return i->rshoul;
        case CONTROLLER_DUP:
            return i->dup;
        case CONTROLLER_DDOWN:
            return i->ddown;
        case CONTROLLER_DLEFT:
            return i->dleft;
        case CONTROLLER_DRIGHT:
            return i->dright;
        case CONTROLLER_LEFTX:
            return smin(i->alx, 0);
        case CONTROLLER_LEFTX_MIN:
            return smax(i->alx, 0);
        case CONTROLLER_RIGHTX:
            return smin(i->arx, 0);
        case CONTROLLER_RIGHTX_MIN:
            return smax(i->arx, 0);
        case CONTROLLER_LEFTY:
            return smin(i->aly, 0);
        case CONTROLLER_LEFTY_MIN:
            return smax(i->aly, 0);
        case CONTROLLER_RIGHTY:
            return smin(i->ary, 0);
        case CONTROLLER_RIGHTY_MIN:
            return smax(i->ary, 0);
        case CONTROLLER_LTRIG:
            return i->altrig;
        case CONTROLLER_RTRIG:
            return i->artrig;
        default:
            dlog("con_get_input(): invalid ButtonAxis value %d", ba);
            return 0;
    }
}

static int16_t get_state_mapping_button(inputs_t *i, ControllerMapping *mapping)
{
    int16_t p = get_state_buttonaxis(i, mapping->primary);
    int16_t s = get_state_buttonaxis(i, mapping->secondary);

    if (mapping->primary >= CONTROLLER_AXIS_BEGIN) {
        p = threshold(p, concfg.a2d_threshold) != 0;
    }

    if (mapping->secondary >= CONTROLLER_AXIS_BEGIN) {
        s = threshold(s, concfg.a2d_threshold) != 0;
    }

    return p || s;
}

static int16_t get_state_mapping_axis(inputs_t *i, ControllerMapping *plus, ControllerMapping *minus)
{
    int16_t plus_p = get_state_buttonaxis(i, plus->primary);
    int16_t plus_s = get_state_buttonaxis(i, plus->secondary);

    int16_t minus_p = get_state_buttonaxis(i, minus->primary);
    int16_t minus_s = get_state_buttonaxis(i, minus->secondary);

    if (plus->primary < CONTROLLER_AXIS_BEGIN) {
        plus_p = plus_p * 32767;
    }

    if (plus->secondary < CONTROLLER_AXIS_BEGIN) {
        plus_s = plus_s * 32767;
    }

    if (minus->primary < CONTROLLER_AXIS_BEGIN) {
        minus_p = minus_p * -32767;
    }

    if (minus->secondary < CONTROLLER_AXIS_BEGIN) {
        minus_s = minus_s * -32767;
    }

    int32_t axis = plus_p + plus_s + minus_p + minus_s;
    
    if (axis > 32767) return 32767;
    if (axis < -32768) return -32768;

    return axis;
}

EXPORT void CALL GetKeys(int Control, BUTTONS *Keys)
{
    inputs_t i = {0};
    con_get_inputs(&i);

    Keys->Value = 0;

    Keys->R_DPAD = get_state_mapping_button(&i, &concfg.dright);
    Keys->L_DPAD = get_state_mapping_button(&i, &concfg.dleft);
    Keys->D_DPAD = get_state_mapping_button(&i, &concfg.ddown);
    Keys->U_DPAD = get_state_mapping_button(&i, &concfg.dup);
    Keys->START_BUTTON = get_state_mapping_button(&i, &concfg.start);
    Keys->Z_TRIG = get_state_mapping_button(&i, &concfg.z);
    Keys->A_BUTTON = get_state_mapping_button(&i, &concfg.a);
    Keys->B_BUTTON = get_state_mapping_button(&i, &concfg.b);

    Keys->R_CBUTTON = get_state_mapping_button(&i, &concfg.cright);
    Keys->L_CBUTTON = get_state_mapping_button(&i, &concfg.cleft);
    Keys->D_CBUTTON = get_state_mapping_button(&i, &concfg.cdown);
    Keys->U_CBUTTON = get_state_mapping_button(&i, &concfg.cup);
    Keys->R_TRIG = get_state_mapping_button(&i, &concfg.r);
    Keys->L_TRIG = get_state_mapping_button(&i, &concfg.l);

    int16_t x = get_state_mapping_axis(&i, &concfg.right, &concfg.left);
    int16_t y = get_state_mapping_axis(&i, &concfg.down, &concfg.up);
    scale_and_limit(&x, &y, concfg.deadzone, concfg.outer_edge);

    n64_analog(
        Keys,
        x,
        y
    );
}

EXPORT void CALL InitiateControllers(HWND hMainWindow, CONTROL Controls[4])
{
    for (int i = 0; i < 4; ++i)
    {
        Controls[i].Present = FALSE;
        Controls[i].RawData = FALSE;
    }
    Controls[0].Present = TRUE;
}

//EXPORT void CALL ReadController(int Control, BYTE * Command) {}

//EXPORT void CALL RomClosed(void) {}

EXPORT void CALL RomOpen(void)
{
    con_open();
}

//EXPORT void CALL WM_KeyDown(WPARAM wParam, LPARAM lParam) {}

//EXPORT void CALL WM_KeyUp(WPARAM wParam, LPARAM lParam) {}
