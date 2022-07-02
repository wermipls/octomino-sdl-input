/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "Input.hpp"

class InputSDL : public Input
{
public:
    InputSDL();
    ~InputSDL();

    std::vector<DeviceInfo> GetDeviceList();
    DeviceState GetDeviceState(int id);
    const std::string GetButtonName(int id);
    const std::string GetAxisName(int id, bool is_positive);
};
