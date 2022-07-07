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
#define BORDER_SIZER 6

class ConfigDialog : public wxDialog
{
public:
    ConfigDialog(wxWindow *parent);

private:
    void OnMappingPrimaryClick(wxCommandEvent &e);
    void control_mapping(wxWindow *parent, wxSizer *sizer, wxString name);
    void control_mapping_box(wxWindow *parent, wxSizer *sizer, int start_id, int end_id, wxString name);
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
    "Up",
    "Down",
    "Left",
    "Right",
};

void ConfigDialog::OnMappingPrimaryClick(wxCommandEvent &e)
{
    auto obj = e.GetEventObject();

    if (obj->IsKindOf(wxCLASSINFO(wxButton)))
    {
        wxButton *b = (wxButton *)obj;

        b->SetLabel("Awaiting input...");

        
    }
}

void ConfigDialog::control_mapping(wxWindow *parent, wxSizer *sizer, wxString name)
{
    auto *text_name = new wxStaticText(parent, wxID_ANY, name, wxDefaultPosition, wxSize(50, -1));
    auto *primary = new wxButton(parent, wxID_ANY, "Not set", wxDefaultPosition, wxSize(128, 22));
    auto *secondary = new wxButton(parent, wxID_ANY, "...", wxDefaultPosition, wxSize(22, 22));

    sizer->Add(text_name, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);
    sizer->Add(primary, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, BORDER_SIZE);
    sizer->Add(secondary, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);

    primary->Bind(wxEVT_BUTTON, &ConfigDialog::OnMappingPrimaryClick, this);
}

void ConfigDialog::control_mapping_box(wxWindow *parent, wxSizer *sizer, int start_id, int end_id, wxString name)
{
    auto *box_mapping = new wxStaticBoxSizer(wxVERTICAL, parent, name);
    sizer->Add(box_mapping, 0, wxALL|wxEXPAND, BORDER_SIZER);

    auto *sizer_mapping = new wxFlexGridSizer(0, 3, 0, 0);
    box_mapping->Add(sizer_mapping, 1, wxALL|wxEXPAND, BORDER_SIZER);
    sizer_mapping->AddGrowableCol(1, 1);

    for (int i = start_id; i < end_id; i++) {
        control_mapping(box_mapping->GetStaticBox(), sizer_mapping, mappings[i]);
    }
}

ConfigDialog::ConfigDialog(wxWindow *parent)
    : wxDialog(parent, wxID_ANY, PLUGIN_NAME " configuration",
               wxDefaultPosition, wxSize(800, 480), wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
    auto *sizer_main = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(sizer_main);

    auto *sizer_devconf = new wxBoxSizer(wxHORIZONTAL);
    sizer_main->Add(sizer_devconf, 0, wxALL|wxEXPAND, BORDER_SIZER);

    // device box
    auto *sizer_device = new wxStaticBoxSizer(wxHORIZONTAL, this, "Device");
    sizer_devconf->Add(sizer_device, 1, wxALL|wxEXPAND, BORDER_SIZER);

    auto *device_combo = new wxComboBox(sizer_device->GetStaticBox(), wxID_ANY);
    auto *device_refresh = new wxButton(sizer_device->GetStaticBox(), wxID_ANY, "Refresh");
    sizer_device->Add(device_combo, 1, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);
    sizer_device->Add(device_refresh, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);

    // reset box
    auto *sizer_reset = new wxStaticBoxSizer(wxHORIZONTAL, this, "Reset");
    sizer_devconf->Add(sizer_reset, 0, wxALL|wxEXPAND, BORDER_SIZER);

    auto *conf_default = new wxButton(sizer_reset->GetStaticBox(), wxID_ANY, "Default");
    sizer_reset->Add(conf_default, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);

    // profile box
    auto *sizer_conf = new wxStaticBoxSizer(wxHORIZONTAL, this, "Profile");
    sizer_devconf->Add(sizer_conf, 0, wxALL|wxEXPAND, BORDER_SIZER);

    auto *conf_load = new wxButton(sizer_conf->GetStaticBox(), wxID_ANY, "Load");
    auto *conf_save = new wxButton(sizer_conf->GetStaticBox(), wxID_ANY, "Save");

    sizer_conf->Add(conf_load, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);
    sizer_conf->Add(conf_save, 0, wxALIGN_CENTER_VERTICAL|wxALL, BORDER_SIZE);

    // main settings
    auto *sizer_settings = new wxBoxSizer(wxHORIZONTAL);
    sizer_main->Add(sizer_settings, 0, wxALL|wxEXPAND, BORDER_SIZER);

    auto *sizer_buttons = new wxBoxSizer(wxVERTICAL);
    auto *sizer_buttons2 = new wxBoxSizer(wxVERTICAL);
    auto *sizer_analog = new wxBoxSizer(wxVERTICAL);
    auto *sizer_other = new wxBoxSizer(wxVERTICAL);

    sizer_settings->Add(sizer_buttons, 1, wxALL|wxEXPAND, 0);
    sizer_settings->Add(sizer_buttons2, 1, wxALL|wxEXPAND, 0);
    sizer_settings->Add(sizer_analog, 1, wxALL|wxEXPAND, 0);
    sizer_settings->Add(sizer_other, 1, wxALL|wxEXPAND, 0);

    control_mapping_box(this, sizer_buttons, 6, 10, "D-Pad");
    control_mapping_box(this, sizer_buttons, 10, 14, "C-Buttons");
    control_mapping_box(this, sizer_buttons2, 0, 6, "Buttons");
    control_mapping_box(this, sizer_analog, 14, 18, "Analog Stick");

    auto *box_options = new wxStaticBoxSizer(wxVERTICAL, this, "Analog Stick settings");
    sizer_analog->Add(box_options, 0, wxALL|wxEXPAND, BORDER_SIZER);

    this->SetSize(-1, -1, -1, -1, wxSIZE_AUTO);
    this->SetMinSize(this->GetSize());
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
