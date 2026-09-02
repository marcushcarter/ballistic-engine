#pragma once
#include <editor/popup/popup.h>

namespace lumen {

struct ProjectSettingsPopup : Popup
{
    const char* name() const override { return "Project Settings"; }
    void draw_contents(EditorContext& ctx) override;
};

}