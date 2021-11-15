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
    x = ((int32_t)x * 80) / 32767;
    y = ((int32_t)y * 80) / 32767;

    int16_t lim_x = 80 - (int16_t)round(abs(sclamp(y, -70, 70)) / 7. - 1. / 7);
    int16_t lim_y = 80 - (int16_t)round(abs(sclamp(x, -70, 70)) / 7. - 1. / 7);

    Keys->Y_AXIS = sclamp(x, -lim_x, lim_x);
    Keys->X_AXIS = -sclamp(y, -lim_y, lim_y);
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

    n64_analog(
        Keys,
        scale_and_limit(i.alx, 0.05f, 0.8f),
        scale_and_limit(i.aly, 0.05f, 0.8f)
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
