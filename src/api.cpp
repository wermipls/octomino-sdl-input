/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <math.h>
#include "zilmar_controller_1.0.h"
#include "util.hpp"
#include "InputSDL.hpp"
#include "config.hpp"

Input *g_input = nullptr;

void input_initialize()
{
    if (g_input != nullptr) return;

    g_input = new InputSDL();
}

void input_deinitialize()
{
    if (g_input == nullptr) return;

    delete g_input;
    g_input = nullptr;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hinstance = hinstDLL;

        InitializeCriticalSection(&g_critical);

        // make a log file
        CreateDirectoryA("Logs", NULL);
        logfile = fopen("Logs\\" PLUGIN_NAME ".txt", "w");

        // make/load a config file
        CreateDirectoryA("Config", NULL);
        config_initialize();

        break;
    case DLL_PROCESS_DETACH:
        fclose(logfile);
        config_deinit();

        DeleteCriticalSection(&g_critical);
        break;
    }
    return TRUE;
}

EXPORT void CALL CloseDLL(void)
{
    DWORD threadid = GetCurrentThreadId();
    dlog("CloseDLL() call from thread %d", threadid);
    input_deinitialize();
}

//EXPORT void CALL ControllerCommand(int Control, BYTE * Command) {}

EXPORT void CALL DllAbout(HWND hParent)
{
    DWORD threadid = GetCurrentThreadId();
    dlog("DllAbout() call from thread %d", threadid);
    MessageBoxA(
        hParent,
        PLUGIN_ABOUT,
        "About " PLUGIN_NAME,
        MB_ICONINFORMATION
    );
}

EXPORT void CALL DllConfig(HWND hParent)
{
    DWORD threadid = GetCurrentThreadId();
    dlog("DllConfig() call from thread %d", threadid);
    input_initialize();
    dlog("...done?");
}

//EXPORT void CALL DllTest(HWND hParent) {}

EXPORT void CALL GetDllInfo(PLUGIN_INFO * PluginInfo)
{
    PluginInfo->Version = 0x0100;
    PluginInfo->Type = PLUGIN_TYPE_CONTROLLER;
    snprintf(
        PluginInfo->Name,
        sizeof(PluginInfo->Name),
        "%s (%s)",
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

static int16_t get_state_buttonaxis(DeviceState *i, ButtonAxisID ba)
{
    if (!ba.is_mapped) return 0;

    if (ba.is_axis) {
        if (ba.is_positive) {
            return smin(i->axis[ba.id], 0);
        } else {
            return smax(i->axis[ba.id], 0);
        }
    } else {
        return i->button[ba.id];
    }
}

static int16_t get_state_mapping_button(DeviceState *i, ButtonAxisMapping *mapping)
{
    int16_t p = get_state_buttonaxis(i, mapping->primary);
    int16_t s = get_state_buttonaxis(i, mapping->secondary);

    if (mapping->primary.is_axis) {
        float t = concfg.a2d_threshold;
        p = threshold(p, t) != 0;
    }

    if (mapping->primary.is_axis) {
        float t = concfg.a2d_threshold;
        s = threshold(s, t) != 0;
    }

    return p || s;
}

static int16_t get_state_mapping_axis(DeviceState *i, ButtonAxisMapping *plus, ButtonAxisMapping *minus)
{
    int16_t plus_p = get_state_buttonaxis(i, plus->primary);
    int16_t plus_s = get_state_buttonaxis(i, plus->secondary);

    int16_t minus_p = get_state_buttonaxis(i, minus->primary);
    int16_t minus_s = get_state_buttonaxis(i, minus->secondary);

    if (!plus->primary.is_axis) {
        plus_p = plus_p * 32767;
    }

    if (!plus->secondary.is_axis) {
        plus_s = plus_s * 32767;
    }

    if (!minus->primary.is_axis) {
        minus_p = minus_p * -32767;
    }

    if (!minus->secondary.is_axis) {
        minus_s = minus_s * -32767;
    }

    int32_t axis = plus_p + plus_s + minus_p + minus_s;
    
    if (axis > 32767) return 32767;
    if (axis < -32768) return -32768;

    return axis;
}

EXPORT void CALL GetKeys(int Control, BUTTONS *Keys)
{
    input_initialize();
    auto i = g_input->GetDeviceState(0);

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
    DWORD threadid = GetCurrentThreadId();
    dlog("RomOpen() call from thread %d", threadid);
    input_initialize();
}

//EXPORT void CALL WM_KeyDown(WPARAM wParam, LPARAM lParam) {}

//EXPORT void CALL WM_KeyUp(WPARAM wParam, LPARAM lParam) {}
