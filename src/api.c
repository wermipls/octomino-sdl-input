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
        configfile = fopen("Config\\" PLUGIN_NAME ".ini", "rb+");
        config_initialize(&concfg);

        break;
    case DLL_PROCESS_DETACH:
        fclose(logfile);
        fclose(configfile);
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
    open_controller();
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
    x = ((int32_t)x * concfg.range) / 32767;
    y = ((int32_t)y * concfg.range) / 32767;

    int diagonal = concfg.range * 70 / 80;
    int16_t lim_x = concfg.range - (int16_t)round(abs(sclamp(y, -diagonal, diagonal)) / 7. - 1. / 7);
    int16_t lim_y = concfg.range - (int16_t)round(abs(sclamp(x, -diagonal, diagonal)) / 7. - 1. / 7);

    if (concfg.is_clamped) {
        if (lim_x < x) {
            x = lim_x;
            y = y * (lim_x / x);
        } else if (lim_y < y) {
            y = lim_y;
            x = x * (lim_y / y);
        }
    }

    Keys->Y_AXIS = x;
    Keys->X_AXIS = -y;
}

EXPORT void CALL GetKeys(int Control, BUTTONS *Keys)
{
    inputs_t i = {0};
    get_inputs(&i);

    Keys->Value = 0;

    Keys->R_DPAD = i.dright;
    Keys->L_DPAD = i.dleft;
    Keys->D_DPAD = i.ddown;
    Keys->U_DPAD = i.dup;
    Keys->START_BUTTON = i.start;
    Keys->Z_TRIG = threshold(i.altrig, 0.25f) > 0;
    Keys->A_BUTTON = i.a || i.b;
    Keys->B_BUTTON = i.x || i.y;

    Keys->R_CBUTTON = threshold(i.arx, 0.25f) > 0;
    Keys->L_CBUTTON = threshold(i.arx, 0.25f) < 0;
    Keys->D_CBUTTON = threshold(i.ary, 0.25f) > 0;
    Keys->U_CBUTTON = threshold(i.ary, 0.25f) < 0;
    Keys->R_TRIG = threshold(i.artrig, 0.25f) > 0 || i.rshoul;
    Keys->L_TRIG = i.lshoul;

    int16_t x = i.alx;
    int16_t y = i.aly;
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
    open_controller();
}

//EXPORT void CALL WM_KeyDown(WPARAM wParam, LPARAM lParam) {}

//EXPORT void CALL WM_KeyUp(WPARAM wParam, LPARAM lParam) {}
