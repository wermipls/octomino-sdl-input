/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "GUI.hpp"
#include "util.hpp"

#pragma GCC diagnostic ignored "-Wdeprecated-copy"
    #include <wx/wxprec.h>
    #ifndef WX_PRECOMP
        #include <wx/wx.h>
        #include <wx/dialog.h>
        #include <wx/sizer.h>
    #endif
#pragma GCC diagnostic pop

#define BORDER_SIZE 2
#define BORDER_SIZER 8

class ConfigDialog : public wxDialog
{
public:
    ConfigDialog(wxWindow *parent);

private:

};

static const wxString mappings[] = {
    "A",
    "B",
    "Z",
    "L",
    "R",
    "Start",
    "D-Up",
    "D-Down",
    "D-Left",
    "D-Right",
    "C-Up",
    "C-Down",
    "C-Left",
    "C-Right",
    "Analog Up",
    "Analog Down",
    "Analog Left",
    "Analog Right",
};

ConfigDialog::ConfigDialog(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, PLUGIN_NAME " configuration",
               wxDefaultPosition, wxSize(600, 600))
{
    auto *sizer_main = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer_main);

    auto *sizer_device = new wxStaticBoxSizer(wxHORIZONTAL, this, "Device");
    sizer_main->Add(sizer_device, 0, wxALL|wxEXPAND, BORDER_SIZER);

    auto *device_combo = new wxComboBox(this, wxID_ANY);
    auto *device_refresh = new wxButton(this, wxID_ANY, "Refresh");

    sizer_device->Add(device_combo, 1, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);
    sizer_device->Add(device_refresh, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);

    auto *sizer_mappings = new wxFlexGridSizer(0, 6, 0, 0);
    sizer_main->Add(sizer_mappings, 0, wxALL|wxEXPAND, BORDER_SIZER);
    sizer_mappings->AddGrowableCol(1, 1);
    sizer_mappings->AddGrowableCol(4, 1);

    for (int i = 0; i < 18; i++) {
        auto *button = new wxStaticText(this, wxID_ANY, mappings[i]);
        auto *primary = new wxButton(this, wxID_ANY, "Not set");
        auto *secondary = new wxButton(this, wxID_ANY, "...", wxDefaultPosition, wxSize(30, -1));

        sizer_mappings->Add(button, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);
        sizer_mappings->Add(primary, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, BORDER_SIZE);
        sizer_mappings->Add(secondary, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);
    }
}

void config_window(HWND hwnd_parent)
{
    new wxApp();
    wxEntryStart(NULL);

    int result;
    {
        wxWindow parent;
        parent.SetHWND((WXHWND)hwnd_parent);
        parent.AdoptAttributesFromHWND();
        wxTopLevelWindows.Append(&parent);

        ConfigDialog dlg(&parent);
        result = dlg.ShowModal();

        wxTopLevelWindows.DeleteObject(&parent);
        parent.SetHWND((WXHWND)NULL);
    }

    wxEntryCleanup();
}
