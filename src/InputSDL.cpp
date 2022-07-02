/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "InputSDL.hpp"
#include "input_sdl.hpp"
#include <stdio.h>
#include <stdexcept>
#include <SDL2/SDL.h>
#include <Shlwapi.h>
#include <synchapi.h>

HANDLE sdl_thread_handle = NULL;
HANDLE sdl_init_finish;
HANDLE terminate_event;

DWORD WINAPI sdl_init_thread(LPVOID lpParam)
{
    int err = try_init();
    SetEvent(sdl_init_finish);

    if (err) {
        return -1;
    }

    WaitForSingleObject(terminate_event, INFINITE);

    deinit();
    return 0;
}

InputSDL::InputSDL()
{
    // we create a thread for handling SDL initialization to guarantee both
    // initialization and deinitialization will happen on the same thread.
    // without that, we can get bunch of weird crashes
 
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

    con_open();
}

InputSDL::~InputSDL()
{
    SetEvent(terminate_event);
    WaitForSingleObject(sdl_thread_handle, INFINITE);
}

std::vector<DeviceInfo> InputSDL::GetDeviceList()
{
    auto list = std::vector<DeviceInfo>();

    con_open();

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

    state.axis = std::vector<int16_t>(SDL_CONTROLLER_AXIS_MAX);

    for (int i = 0; i < SDL_CONTROLLER_AXIS_MAX; i++) {
        auto a = (SDL_GameControllerAxis)i;
        state.axis[i] = SDL_GameControllerGetAxis(con, a);
    }

    state.button = std::vector<bool>(SDL_CONTROLLER_BUTTON_MAX);

    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
        auto b = (SDL_GameControllerButton)i;
        state.button[i] = SDL_GameControllerGetButton(con, b);
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
