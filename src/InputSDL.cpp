/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "InputSDL.hpp"
#include "util.hpp"
#include <stdio.h>
#include <stdexcept>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <Shlwapi.h>
#include <synchapi.h>

HANDLE sdl_thread_handle = NULL;
HANDLE sdl_init_finish = NULL;
HANDLE terminate_event = NULL;

char dbpath[PATH_MAX];
int initialized = 0;
SDL_GameController *con = NULL;
int joy_inst = -1;

void con_close(void)
{
    if (!initialized && con != NULL) con = NULL;
    if (!initialized || con == NULL) {
        return;
    }

    dlog("Closing current controller");
    SDL_GameControllerClose(con);
    con = NULL;
    joy_inst = -1;
}

void con_open(void)
{
    dlog("Attempting to open a controller");

    if (!initialized) {
        dlog("Failed to open a controller: SDL not initialized");
        return;
    }

    if (con != NULL) {
        dlog("Failed to open a controller: controller is not null");
        return;
    }

    dlog("    # of joysticks: %d", SDL_NumJoysticks());

    // open the first available controller
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if (SDL_IsGameController(i) && (con = SDL_GameControllerOpen(i)) != NULL)
        {
            dlog("    Found a viable controller: %s (joystick %d)", SDL_GameControllerName(con), i);

            SDL_Joystick *joy = SDL_GameControllerGetJoystick(con);
    
            joy_inst = SDL_JoystickInstanceID(joy);
            dlog("        Joystick instance ID: %d", joy_inst);

            SDL_JoystickGUID guid = SDL_JoystickGetGUID(joy);
            char guidstr[33];
            SDL_JoystickGetGUIDString(guid, guidstr, sizeof(guidstr));
            dlog("        Joystick GUID: %s", guidstr);

            char *mapping = SDL_GameControllerMapping(con);
            if (mapping != NULL) {
                dlog("        Controller mapping: %s", mapping);
                SDL_free(mapping);
            } else {
                dlog("        This controller has no mapping! Closing it");
                // skip this controller
                con_close();
                continue;
            }

            break;
        }
        else
            dlog("    Couldn't use joystick %d", i);
    }

    if (con == NULL)
        dlog("    Couldn't find a viable controller :(");    
}

int try_init(void)
{
    if (initialized) {
        dlog("Attempted initialize, but SDL is already initialized");
        return -1;
    }
    dlog("Initializing");

    GetModuleFileNameA(g_hinstance, dbpath, sizeof(dbpath));
    PathRemoveFileSpecA(dbpath);
    PathCombineA(dbpath, dbpath, "gamecontrollerdb.txt");

    SDL_SetMainReady();
    if (!SDL_Init(SDL_INIT_GAMECONTROLLER))
    {
        /* deal with the unnessessary initial controller connected
           events so they don't clog up the log file */
        SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

        int mapcount = SDL_GameControllerAddMappingsFromFile(dbpath);
        if (mapcount == -1)
            dlog("    Unable to load mappings from %s", dbpath);
        else
            dlog("    Successfully loaded %d mappings from %s", mapcount, dbpath);

        initialized = 1;
        dlog("    ...done");
    }
    else {
        dlog("    SDL has failed to initialize");
        return -2;
    }

    con_open();

    return 0;
}

void deinit(void)
{
    if (!initialized) {
        return;
    }

    dlog("Deinitializing");

    con_close();
    SDL_Quit();
    initialized = 0;
}

DWORD WINAPI sdl_init_thread(LPVOID lpParam)
{
    int err = try_init();
    SetEvent(sdl_init_finish);

    if (err) {
        return -1;
    }

    while (WaitForSingleObject(terminate_event, 1) == WAIT_TIMEOUT)
    {
        // SDL 2.0.14: fix for SDL_CONTROLLERDEVICEADDED events not showing up
        // see https://github.com/libsdl-org/SDL/issues/4147 for details
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            DispatchMessageW(&msg);
        }

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type)
            {
            case SDL_CONTROLLERDEVICEADDED:
                dlog("A device has been added");
                if (con == NULL)
                {
                    dlog("    ...and there is no active controller");
                    con_open();
                }
                else
                    dlog("    ...but there is already an active controller");
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                dlog("A device has been removed");
                if (e.cdevice.which == joy_inst)
                {
                    dlog("    ...it was the active controller");
                    con_close();
                    con_open();
                }
                else
                    dlog("    ...it was not the active controller");
                break;
            }
        }
    }

    deinit();
    return 0;
}

InputSDL::InputSDL()
{
    // we create a thread for handling SDL initialization to guarantee
    // (de)initialization and event handling all happen on the same thread.
    // without that, we can get bunch of weird crashes or other problems
 
    if (terminate_event == NULL) {
        terminate_event = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    if (sdl_init_finish == NULL) {
        sdl_init_finish = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    sdl_thread_handle = CreateThread(NULL, 0, sdl_init_thread, NULL, 0, NULL);
    if (sdl_thread_handle == NULL) {
        throw std::runtime_error("Failed to create SDL thread");
    }

    WaitForSingleObject(sdl_init_finish, INFINITE);

    if (!initialized) {
        WaitForSingleObject(sdl_thread_handle, INFINITE);
        throw std::runtime_error("Failed to initialize SDL");
    }
}

InputSDL::~InputSDL()
{
    SetEvent(terminate_event);
    WaitForSingleObject(sdl_thread_handle, INFINITE);
}

std::vector<DeviceInfo> InputSDL::GetDeviceList()
{
    auto list = std::vector<DeviceInfo>();

    if (con) {
        auto joy = SDL_GameControllerGetJoystick(con);
        auto guid = SDL_JoystickGetGUID(joy);

        DeviceInfo device;
        memcpy(&device.id, &guid, 16);
        device.name = SDL_GameControllerName(con);

        list.push_back(device);
    }

    return list;
}

DeviceState InputSDL::GetDeviceState(int id)
{
    DeviceState state;

    if (con != NULL) {
        for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; i++) {
            auto a = (SDL_GameControllerAxis)i;
            state.axis[i] = SDL_GameControllerGetAxis(con, a);
        }

        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
            auto b = (SDL_GameControllerButton)i;
            state.button[i] = SDL_GameControllerGetButton(con, b);
        }
    }

    return state;
}

const std::string InputSDL::GetButtonName(int id)
{
    auto b = (SDL_GameControllerButton)id;
    return SDL_GameControllerGetStringForButton(b);
}

const std::string InputSDL::GetAxisName(int id, bool is_positive)
{
    auto a = (SDL_GameControllerAxis)id;
    std::string name = SDL_GameControllerGetStringForAxis(a);

    if (is_positive) {
        name += "+";
    } else {
        name += "-";
    }

    return name;
}
