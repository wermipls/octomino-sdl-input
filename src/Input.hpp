/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <vector>
#include <string>

struct ControllerID
{
    uint8_t data[16];
};

struct DeviceInfo 
{
    ControllerID id; 
    std::string name;
    bool is_plugged_in;
};

struct DeviceState
{
    bool button[128] = {0};
    int16_t axis[8] = {0};
};

class Input
{
public:
    virtual ~Input() {};

    virtual std::vector<DeviceInfo> GetDeviceList() = 0;
    virtual DeviceState GetDeviceState(int id) = 0;
    virtual const std::string GetButtonName(int id) = 0;
    virtual const std::string GetAxisName(int id, bool is_positive) = 0;
};